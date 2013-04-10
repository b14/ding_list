<?php
/**
 * @file
 * Functions handling the create, edit and delete api.
 */

/**
 * Get lists connected to a user.
 *
 * @param $list_type
 *   The list type to filter. If you supply an array it will filter by all the
 *   lists in that array (using OR).
 *   Combining this with the ding_list_get_list_types_by_operations(), will
 *   give an easy way of selecting specific lists which can do specific
 *   operations.
 * @param $account
 *   The user account on who you want to get the reservations from (if none is
 *   given it will default to the current user).
 * @param $range
 *   From where to where, in an array(from, length).
 * @param $order_by
 *   Order by in an array ('what', 'direction'), is ('changed', 'DESC') by
 *   default. Note that you can only order by property.
 *
 * @return
 *   An array of ding_list entities. FALSE otherwise.
 */
function ding_list_get_lists($list_type = NULL, $account = NULL, $range = array(), $order_by = array('changed', 'DESC')) {
  if ($account === NULL) {
    global $user;
    $account = $user;
  }

  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'ding_type')
    ->entityCondition('bundle', 'ding_list')
    ->propertyCondition('uid', $account->uid)
    ->propertyOrderBy($order_by[0], $order_by[1]);

  if ($list_type !== NULL) {
    $query->fieldCondition('field_list_type', 'value', $list_type);
  }

  if (count($range) == 2) {
    $query->range($range[0], $range[1]);
  }

  $result = $query->execute();
  if (!empty($result)) {
    return entity_load('ding_type', array_keys($result['ding_type']));
  }

  return FALSE;
}

/**
 * Create a ding_list_element, and attach it to the list.
 *
 * A ding_list_element should not exist without it being attached to a list,
 * which is why the create function will automatically attach it to a list.
 *
 * @param $list
 *   The ding_list object, which the newly created ding_list_element is
 *   attached to. By reference, so you don't need to update the list after
 *   the element has been added.
 * @param $type
 *   Type of the ding_list_element. This is put into the field_element_type,
 *   and determines which field_[TYPE] to save the $id in.
 * @param $id
 *   The id of the element. This is a string so the id can be a ting_object
 *   id. It can also be a simple string, which is useful when adding a search
 *   query for example.
 *
 * @return
 *   The ding_list_element object.
 */
function ding_list_create_ding_list_element(&$list, $type, $id, $account = NULL, $save_list = TRUE, $openlist_id = FALSE) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  $element = entity_create('ding_type', array(
    'type' => 'ding_list_element',
    'title' => '[' . $list->id . ']' . $list->title . ': ' . $type . ' [' . $id . ']'
  ));

  // The entity_create automatically set the entity uid to the current user so
  // we only need to change this if specified.
  if ($account !== NULL) {
    $element->uid = $account->uid;
  }

  // Set the type, and let the type select which field to add the id to.
  $element->field_element_type[LANGUAGE_NONE][] = array('value' => $type);
  $element->{'field_' . $type}[LANGUAGE_NONE][] = array('value' => $id);

  if (module_exists('ting_openlist') && $openlist_id === FALSE) {
    try {
      $openlist_id = ding_provider_invoke('openlist', 'request', 'createElement', array($list->openlist_id, array('id' => $id, 'type' => $type)));
    } catch (Exception $e) {
      $transaction->rollback();
      drupal_set_message(t("An error occurred, your element wasn't created. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  if ($openlist_id !== FALSE) {
    $element->openlist_id = $openlist_id;
  }

  $element->save();

  // Finaly add the element to the list.
  $list->field_list_objects[LANGUAGE_NONE][] = array(
    'target_id' => $element->id
  );
  if ($save_list === TRUE) {
    $list->save();
  }

  return $element;
}

/**
 * Create a ding_list.
 *
 * Gives a cleaner, not so specific code, at the cost of some performance. But
 * creating lists shouldn't be an often used function anyway.
 *
 * @param $title
 *   The title of the list.
 * @param $type
 *   Type of the list.
 * @param $account
 *   A user object, to connect the list to. If nothing is supplied it will use
 *   the currently logged in user.
 */
function ding_list_create_ding_list($title, $type = DING_LIST_TYPE_USER_LIST, $account = NULL, $more = NULL, $openlist_id = FALSE, $fields = array()) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  $list = entity_create('ding_type', array(
    'type' => 'ding_list',
    'title' => $title
  ));

  // The entity_create automatically set the entity uid to the current user so
  // we only need to change this if specified.
  if ($account !== NULL) {
    $list->uid = $account->uid;
  }

  $list->field_list_type[LANGUAGE_NONE][0]['value'] = $type;

  if ($more !== NULL) {
    $list->more = $more;
  }
  elseif (isset($list->more)) {
    $more = $list->more;
  }

  if (!empty($fields)) {
    foreach ($fields as $field) {
      $list->{$field['name']}[LANGUAGE_NONE] = array(array($field['value_type'] => $field['value']));
    }
  }

  if (module_exists('ting_openlist') && $openlist_id === FALSE) {
    try {
      $openlist_id = ding_provider_invoke('openlist', 'request', 'createList', array(ting_openlist_user_identifier($account), $title, $type, array('more' => $more, 'fields' => $fields)));
    } catch (Exception $e) {
      $transaction->rollback();
      drupal_set_message(t("An error occurred, your list wasn't created. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  if ($openlist_id !== FALSE) {
    $list->openlist_id = $openlist_id;
  }

  $list->save();

  return $list;
}

/**
 * Edit an existing ding_list.
 *
 * @param $list
 *   The list object.
 * @param $title
 *   The new title of the list.
 * @param $more
 *
 * @param $tell_openlist
 *   If this is true (as by default) it will tell the openlist service about
 *   the changes. When synchronizing this would be set to false.
 *
 * @return
 *   The ding_list object
 */
function ding_list_edit_ding_list(&$list, $title, $more = NULL, $tell_openlist = TRUE, $fields = array()) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  if ($list->title === $title && ($more === NULL || $list->more === $more) && empty($fields)) {
    return $list;
  }

  $list->title = $title;

  if ($more !== NULL) {
    $list->more = $more;
  }

  if (!empty($fields)) {
    foreach ($fields as $field) {
      $list->{$field['name']}[LANGUAGE_NONE] = array(array($field['value_type'] => $field['value']));
    }
  }

  if (module_exists('ting_openlist') && $tell_openlist === TRUE && $list->openlist_id > 0) {
    try {
      ding_provider_invoke('openlist', 'request', 'editList', array(
        $list->openlist_id,
        $list->title,
        array(
          'more' => $list->more,
          'fields' => $fields
        )
      ));
    } catch (Exception $e) {
      $transaction->rollback();
      drupal_set_message(t("An error occurred while changing your list. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  $list->save();

  return $list;
}

/**
 * Detach a ding_list_element from a list, and delete the ding_list_element
 * object.
 *
 * @param $list
 *   ding_list object, the ding_list_element is attached to. This is
 *   parsed by reference so you don't need to reload it after the update.
 * @param $element_ids
 *   Array of ding_list_element ids to remove and detach.
 *
 * @return
 *   The count of element items actually deleted. Or FALSE if something went
 *   wrong
 */
function ding_list_detach_ding_list_element(&$list, $element_ids = array(), $save_list = TRUE, $tell_openlist = TRUE) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  if (empty($element_ids)) {
    return 0;
  }

  // Get the ding_list_elements attached to the ding_list.
  $items = ding_base_get_items('ding_type', $list, 'field_list_objects');

  // Run through the ding_list_elements, and determine if they should be
  // removed or kept.
  // This way we can utilize the entity_delete_multiple instead of running a
  // single entity_delete on every id.
  $kept = $delete = array();
  foreach ($items as $item) {
    if (in_array($item['target_id'], $element_ids)) {
      $delete[] = $item['target_id'];
    }
    else {
      $kept[] = $item;
    }
  }

  if (module_exists('ting_openlist') && $tell_openlist === TRUE) {
    $query = db_select('eck_ding_type', 'e');

    $result = $query
      ->fields('e', array('openlist_id'))
      ->condition('id', $element_ids)
      ->execute();

    $openlist_ids = array();
    foreach ($result as $row) {
      $openlist_ids[] = $row->openlist_id;
    }

    try {
      ding_provider_invoke('openlist', 'request', 'deleteElement', array($openlist_ids));
    } catch (Exception $e) {
      $transaction->rollback();
      // drupal_set_message(t("An error occurred while removing your element. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  // Delete the ding_list_elements.
  entity_delete_multiple('ding_type', $delete);

  // Overwrite the old field_list_objects with the new $kept list.
  $list->field_list_objects[LANGUAGE_NONE] = $kept;

  if ($save_list) {
    $list->save();
  }

  return count($delete);
}

/**
 * Delete a list and all it's attached elements.
 *
 * @param $list
 *   The ding_list to delete.
 *   If this is a numeric value, it will automatically load the ding_list
 *   object.
 *   (note that after the function has been run your number will be recasted
 *   as an Entity object).
 */
function ding_list_delete_list($list, $tell_openlist = TRUE) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  // Auto load the ding_list if $list is a number.
  if (is_numeric($list)) {
    $list = entity_load_single('ding_type', $list);
  }

  // Get a list of all the attached elements, and detach them.
  // Detach also deletes the element.
  $keys = array();
  $items = ding_base_get_items('ding_type', $list, 'field_list_objects');
  foreach ($items as $item) {
    $keys[] = $item['target_id'];
  }

  if (!empty($keys)) {
    ding_list_detach_ding_list_element($list, $keys, TRUE, $tell_openlist);
  }

  if (module_exists('ting_openlist') && $tell_openlist === TRUE && $list->openlist_id > 0) {
    try {
      ding_provider_invoke('openlist', 'request', 'deleteList', array($list->openlist_id));
    } catch (Exception $e) {
      $transaction->rollback();
      drupal_set_message(t("An error occurred while deleting your list. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  // Remove the actual ding_list.
  entity_delete('ding_type', $list->id);
}

/**
 * Load an array with the elements
 *
 * @param $list
 *   The ding_list entity.
 *
 * @return
 *   An array with the elements in, or an empty array, if no elements exist.
 */
function ding_list_load_elements($list) {
  if ($list) {
    $items = field_get_items('ding_type', $list, 'field_list_objects');
  }

  if ($items !== FALSE) {
    // Run through all the elements to get the target ids.
    // This way we can load all the objects with a single call instead of
    // loading them lazy.
    $keys = array();
    foreach ($items as $item) {
      $keys[] = $item['target_id'];
    }

    // Return all the elements.
    return entity_load('ding_type', $keys);
  }

  return array();
}

/**
 * Get a list object.
 *
 * @param $list
 *   The value you which to get a list from.
 * @param $account
 *   The user which the list is connected to.
 *   This only affects the result if the $list is a DING_LIST_TYPE.
 *
 * @return
 *   Either FALSE or the list object.
 */
function ding_list_get_list($list, $account = NULL) {
  // If it's a number we assume it's the ID of the list.
  if (is_numeric($list)) {
    return entity_load_single('ding_type', $list);
  }

  // If it's an object return that object.
  if (is_object($list)) {
    return $list;
  }

  // If it's a string, we assume it's a DING_LIST_TYPE, and we grab the first.
  if (is_string($list)) {
    $list_element = ding_list_get_lists($list, $account, array(0, 1));
    if ($list_element !== FALSE) {
      return array_shift($list_element);
    }
  }

  return FALSE;
}

/**
 * Edit an existing ding_list_element.
 *
 * @param $element
 *
 * @param $type
 *   The new type of the element
 * @param $id
 *   The new element id
 * @param $tell_openlist
 *   If this is true (as by default) it will tell the openlist service about
 *   the changes. When synchronizing this would be set to false.
 *
 * @return
 *   The ding_list_element
 */
function ding_list_edit_ding_list_element(&$element, $type = NULL, $id = NULL, $tell_openlist = TRUE) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();

  $old_type = ding_base_get_value('ding_type', $element, 'field_element_type');
  if ($type !== NULL) {
    $element->field_element_type[LANGUAGE_NONE][] = array('value' => $type);
  }
  else {
    $type = $old_type;
  }

  $old_id = ding_base_get_value('ding_type', $element, 'field_' . $type);
  if ($id !== NULL) {
    $element->{'field_' . $type}[LANGUAGE_NONE][] = array('value' => $id);
  }
  else {
    $id = $old_id;
  }

  // Don't save any changes to the database, if the new values are the same
  // as the old ones.
  if ($old_type === $type && $old_id === $id) {
    return $element;
  }

  if (module_exists('ting_openlist') && $tell_openlist === TRUE && $element->openlist_id > 0) {
    try {
      ding_provider_invoke('openlist', 'request', 'editElement', array($element->openlist_id, array('id' => $id, 'type' => $type)));
    } catch (Exception $e) {
      $transaction->rollback();
      drupal_set_message(t("An error occurred while editing your element. Please contact the administrator if this problem persists."), 'error');
      return FALSE;
    }
  }

  $element->save();

  return $element;
}