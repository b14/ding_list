<?php
// @todo PBJ
//   Opdel denne fil i flere filer, som f.eks.
//     - functions.inc : (denne fil) requires de andre, og har default functioner, som ding_list_allowed())
//     - functions.handler.inc : har alle entity wrapper funktionerne, som get, create, delete og edit
//

require_once(dirname(__FILE__) . '/functions.openlist.inc');
require_once(dirname(__FILE__) . '/functions.handler.inc');

// @todo ALL (pbj spørger).
//   Har vi brug for denne funktionalitet, eller er det overflødigt?
/**
 * Get the list types which has a set of specific operations available.
 *
 * @param $access
 *   An array of operations.
 */
function ding_list_get_list_types_by_operations($access) {
  global $list_operations;

  if (!is_array($access)) {
    $access = array($access);
  }

  $count = count($access);
  $list_types = NULL;
  foreach ($list_operations as $list => $operations) {
    if (count(array_intersect($access, $operations)) === $count) {
      $list_types[] = $list;
    }
  }

  return $list_types;
}

/**
 *
 * Fetch some random ding_object from user list
 *
 * @todo fetch multiple
 *
 */
function ding_list_get_random_user_list_item() {

  $lists = ding_list_get_lists(DING_LIST_TYPE_USER_LIST);

  // No lists no result
  if ($lists === FALSE) {
    return false;
  }

  shuffle($lists);

  // Find a non empty list
  foreach($lists as $list) {
    $items = field_get_items('ding_type', $list, 'field_list_objects');
    if (isset($items[0]['target_id'])) {
      $element = entity_load_single('ding_type', $items[0]['target_id']);
      $element_id = b14_tools_get_value('ding_type', $element, 'field_ting_object');
      $ting_entity = ting_object_load($element_id);
      return $ting_entity;
    }
  }
  return false;

}

/**
 * Make sure you're allowed to perform a certain action on a list.
 *
 * @param $list
 *   The list you want to operate on.
 *   If this is a numeric value, it will automatically load the ding_list
 *   object.
 *   (note that after the function has been run your number will be recasted
 *   as an Entity object).
 * @param $operation
 *   The operation you wish to perform.
 */
function ding_list_allowed(&$list, $operation) {
  global $list_operations;

  if (is_numeric($list)) {
    $list = entity_load_single('ding_type', $list);
  }

  return in_array($operation, $list_operations[b14_tools_get_value('ding_type', $list, 'field_list_type')]);
}

/**
 * Load keys from elements in a list of type $key_type / msd
 *
 * @param $user
 *    User account.
 * @param $lid
 *    ID of user list OR list identifier (ie "follow",...)
 * @param $key_type
 *    optional type of key to extract
 */
function ding_list_load_keys($user, $lid, $key_type = FALSE) {
  $keys = array();
  if (is_numeric($lid)) {
    // Load certain list
    $list = current(entity_load('ding_type', array($lid)));
  }
  else {
    // Load named list
    $lists = ding_list_get_lists($lid, $user, array(0, 1));
    if ($lists === FALSE) {
      return array();
    }
    $list = current($lists);
  }

  // Load list elements
  $items = field_get_items('ding_type', $list, 'field_list_objects');
  if ($items !== FALSE) {
    $keys = array();
    foreach ($items as $item) {
      $keys[] = $item['target_id'];
    }

    // Load list entities
    $elements = entity_load('ding_type', $keys);
    $keys = array();
    foreach ($elements as $element) {
      $type = b14_tools_get_value('ding_type', $element, 'field_element_type');
      if ($type !== FALSE && (!$key_type || $type === 'taxonomy_term' ) ) {
        $element_id = b14_tools_get_value('ding_type', $element, 'field_' . $type);
        $keys[] = $element_id;
      }
    }
  }
  return $keys;
}

/**
 * Check to see if a list has a specific element.
 *
 * @param $list
 *   The ding_list to delete.
 *   If this is a numeric value, it will automatically load the ding_list
 *   object.
 *   (note that after the function has been run your number will be recasted
 *   as an Entity object).
 * @param $type
 *   Type of the ding_list_element. This is put into the field_element_type,
 *   and determines which field_[TYPE] to save the $id in.
 * @param $id
 *   The id of the element. This is a string so the id can be a ting_object
 *   id. It can also be a simple string, which is useful when adding a search
 *   query for example.
 */
function ding_list_has_ding_list_element(&$list, $type, $id) {
  // Auto load the ding_list if $list is a number.
  if (is_numeric($list)) {
    $list = entity_load_single('ding_type', $list);
  }
  
  // Get the elements in the list.
  $elements = ding_list_load_elements($list);

  foreach ($elements as $element) {
    $element_type = b14_tools_get_value('ding_type', $element, 'field_element_type');
    if ($element_type !== FALSE && $element_type === $type) {
      $element_id = b14_tools_get_value('ding_type', $element, 'field_' . $element_type);
      if ($element_id !== FALSE && $element_id === $id) {
        return $element;
      }
    }
  }
  
  return FALSE;
}

/**
 * Move an element in a list down below another element.
 *
 * @param $list
 *   If this is a numeric value, it will automatically load the ding_list
 *   object.
 *   (note that after the function has been run your number will be recasted
 *   as an Entity object).
 * @param $element
 *   The element you want to move.
 * @param $previous
 *   The element to position the element being moved after. If this is false
 *   it will move the element to the first position of the list.
 * @param $tell_openlist
 *   If this is true (as by default) it will tell the openlist service about
 *   the changes. When synchronizing this would be set to false.
 *
 */
function ding_list_set_element_position(&$list, $element, $previous = FALSE, $tell_openlist = TRUE) {
  // Auto load the ding_list if $list is a number.
  if (is_numeric($list)) {
    $list = entity_load_single('ding_type', $list);
  }
  
  if (!$previous) {
    $previous = new stdClass();
    $previous->id = 0;
    $previous->openlist_id = 0;
  }
  
  
  $items = b14_tools_get_items('ding_type', $list, 'field_list_objects');
  $new_order = array();
  foreach ($items as $key => $item) {
    if ($item['target_id'] !== $element->id) {
      $new_order[] = $item;
    }
    
    if ($item['target_id'] === $previous->id) {
      $new_order[] = array('target_id' => $element->id);
    }
  }
  if ($previous->id == 0) {
    array_unshift($new_order, array('target_id' => $element->id));
  }
  
  if ($new_order != $items) {
  
    if (module_exists('ting_openlist') && $tell_openlist === TRUE) {
      ting_openlist_request('setElementAfter', array($element->openlist_id, $previous->openlist_id));
    }
  
    $list->field_list_objects[LANGUAGE_NONE] = $new_order;
    $list->save();
  }
}

/**
 * (Can't tell openlist about the changes yet, so don't use)
 *
 * @todo Få den til at snakke sammen med openlist. Enten ved at:
 *         - I denne funktion skal den finde ud af hvad der er rykket og hvor
 *           det er rykket hen. Og så send et setElementAfter kald for hvert
 *           ryk der er registeret.
 *         - Lav en openlist funktion, der kan håndtere en fuld liste, og så
 *           kan sætte weight og previous korrekt.
 */
function ding_list_set_order(&$list, $order) {
  // Auto load the ding_list if $list is a number.
  if (is_numeric($list)) {
    $list = entity_load_single('ding_type', $list);
  }
  
  $element_map = ding_list_create_dinglist_map('ding_list_element', $order);
  
  $old_order = b14_tools_get_items('ding_type', $list, 'field_list_objects');
  
  $list->field_list_objects[LANGUAGE_NONE] = array();
  foreach ($order as $item) {
    $list->field_list_objects[LANGUAGE_NONE][] = array('target_id' => $item);
  }
  
  if ($old_order != $list->field_list_objects[LANGUAGE_NONE]) {
    $list->save();
  }
  
  return $list;
}

/**
 * Create all missing lists with the autocreate option, for a specific user.
 *
 * @param $account
 *   The acount to autocreate lists for.
 */
function ding_list_setup_autocreated($account = NULL) {
  global $list_operations;

  // Get all the list types with autocreate.
  $auto_create = ding_list_get_list_types_by_operations(DING_LIST_OPERATION_AUTO_CREATE);
  $lists = ding_list_get_lists($auto_create, $account);
  $auto_create = array_flip($auto_create);
  
  // Run through the already created lists, and exclude them from the
  // $auto_create variable.
  if ($lists !== FALSE) {
    foreach ($lists as $list) {
      $type = b14_tools_get_value('ding_type', $list, 'field_list_type');
      unset($auto_create[$type]);
    }
  }
  
  // Create all the lists in the $auto_create array.
  foreach ($auto_create as $type => $nothing) {
    ding_list_create_ding_list($list_operations[$type]['_data']['default_title'], $type, $account);
  }
}

/**
 * Update the users historical loans.
 *
 * @param $account
 *   The acount to autocreate lists for.
 */
function ding_list_update_historical_loans($account = NULL) {
  if ($account === NULL) {
    global $user;
    $account = user_load($user->uid);
  }

  // Get the historical loan list.
  $list = ding_list_get_list(DING_LIST_TYPE_LOAN_HISTORY, $account);
    
  $saveList = FALSE;
  if(ding_user_is_provider_user($account)) {
    try {
      $creds = ding_user_get_creds($account);
      if (!empty($creds['name'])) {
        // Get the last update from the list, or set it to 0 if it doesn't exist.
        $latest = is_numeric($list->more) ? $list->more : 0;
        // Create a timestamp representing today.
        $today = mktime(0, 0, 0, date('n'), date('j')); // $yesterday = mktime(0, 0, 0, date('n'), date('j') - 1);
        
        // Call the alma server, and get the historical XML.
        $alma = alma_client();
        $doc = $alma->request('patron/loans/historical', array('borrCard' => $creds['name'], 'fromDate' => date('Y-m-d', $latest)));
        
        foreach ($doc->getElementsByTagName('catalogueRecord') as $item) {
          $loanDate = strtotime($item->parentNode->getAttribute('loanDate'));
          // We only add loans from yesterday.
          if ($loanDate < $today) {
            $saveList = TRUE;
            $ding_entity_id = variable_get('ting_agency', '') . ':' . $item->getAttribute('id');
            ding_list_create_ding_list_element($list, 'ting_object', $ding_entity_id, $account, FALSE);
            $latest = max($latest, $loanDate);
          }
        }
      }
    }
    catch (Exception $e) {
      return t('Sorry but no list is available at this time');
    }
  }
  
  if ($saveList) {
    // Set the latest time to tomorrow.
    $latest = mktime(0, 0, 0, date('n', $latest), date('j', $latest) + 1, date('Y', $latest));
    ding_list_edit_ding_list($list, $list->title, $latest);
  }
}
