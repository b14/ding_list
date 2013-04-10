<?php
/**
 * @file
 * API Functions.
 */
 
require_once __DIR__ . '/functions.openlist.inc';
require_once __DIR__ . '/functions.handler.inc';

function ding_list_init() {
  global $user;
  
  if ($user->uid == 3 && request_uri() == '/user/manage-lists') {
    // ding_list_reset_user();
  }
}

/**
 * Reset a user.
 *
 * It's important to note, that this will also clear the users lists on
 * openlist, if the openlist has the ResetUser module.
 *
 * This will delete all lists.
 */
function ding_list_reset_user($account = NULL) {
  if ($account === NULL) {
    global $user;
    $account = $user;
  }
  
  $lists = ding_list_get_lists(NULL, $account);
  if ($lists !== FALSE) {
    foreach ($lists as $list) {
      ding_list_delete_list($list, FALSE);
      b14dpm(3, $list);
    }
  }
  
  if (module_exists('ting_openlist')) {
    try {
      $result = ding_provider_invoke('openlist', 'request', 'callModule', array('ResetUser', 'reset', array(ting_openlist_user_identifier($account))));
    } catch (Exception $e) {
    }
    ding_list_reset_openlist_modified($account);
  }
}


/**
 * Get the list types which has a set of specific operations available.
 *
 * @param $access
 *   An array of operations.
 *
 * @return
 *   The list types having the given operations.
 */
function ding_list_get_list_types_by_operations($access) {
  $list_operations = ding_list_list_operations();

  if (!is_array($access)) {
    $access = array($access);
  }

  $count = count($access);
  $list_types = array();
  foreach ($list_operations as $list => $operations) {
    if (count(array_intersect($access, $operations)) === $count) {
      $list_types[] = $list;
    }
  }

  return $list_types;
}

/**
 * Fetch some random ding_object from user list
 *
 * @return
 *   The random ting_object. Or FALSE otherwise.
 */
function ding_list_get_random_user_list_item() {

  // Select among these types
  $lists = ding_list_get_lists(array(
    DING_LIST_TYPE_USER_LIST,
    DING_LIST_TYPE_LOAN_HISTORY,
    DING_LIST_TYPE_BOOKS_READ,
    DING_LIST_TYPE_REMEMBER
  ));

  // No lists no result
  if ($lists === FALSE) {
    return FALSE;
  }

  shuffle($lists);

  // Find a non empty list
  foreach ($lists as $list) {
    $items = field_get_items('ding_type', $list, 'field_list_objects');    
    if (isset($items[0]['target_id'])) {
      $index = rand(0,count($items)-1);
      $element = entity_load_single('ding_type', $items[$index]['target_id']);
      $element_id = ding_base_get_value('ding_type', $element, 'field_ting_object');
      $ting_entity = ting_object_load($element_id);
      // Return as soon as we have found
      return $ting_entity;
    }
  }
  return FALSE;

}

/**
 * Make sure you're allowed to perform a certain action on a list.
 *
 * @param $list
 *   The list you want to operate on.
 * @param $operation
 *   The operation you wish to perform.
 *
 * @return
 *   TRUE if the list has the operation, and FALSE if it doesn't.
 */
function ding_list_allowed($list, $operation) {
  $list_operations = ding_list_list_operations();

  return in_array($operation, $list_operations[ding_base_get_value('ding_type', $list, 'field_list_type')]);
}

/**
 * Load keys from elements in a list of type $key_type / msd
 *
 * @param $user
 *   User account.
 * @param $lid
 *   ID of user list OR list identifier (ie "follow",...)
 * @param $key_type
 *   optional type of key to extract
 *
 * @return
 *   An array of keys.
 */
function ding_list_load_keys($user, $lid, $key_type = FALSE) {
  $keys = array();
  if (is_numeric($lid)) {
    // Load certain list
    $list = entity_load('ding_type', array($lid));
    if ($list === FALSE) {
      return array();
    }
    $list = array_shift($list);
  }
  else {
    // Load named list
    $lists = ding_list_get_lists($lid, $user, array(0, 1));
    if ($lists === FALSE) {
      return array();
    }
    $list = array_shift($lists);
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
      $type = ding_base_get_value('ding_type', $element, 'field_element_type');
      if ($type !== FALSE && (!$key_type || $type === 'taxonomy_term' ) ) {
        $element_id = ding_base_get_value('ding_type', $element, 'field_' . $type);
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
 * @param $type
 *   Type of the ding_list_element. This is put into the field_element_type,
 *   and determines which field_[TYPE] to save the $id in.
 * @param $id
 *   The id of the element. This is a string so the id can be a ting_object
 *   id. It can also be a simple string, which is useful when adding a search
 *   query for example.
 *
 * @return
 *   If the list has the element, return that element entity, if not return
 *   FALSE
 */
function ding_list_has_ding_list_element($list, $type, $id) {
  // Get the elements in the list.
  $elements = ding_list_load_elements($list);

  foreach ($elements as $element) {
    $element_type = ding_base_get_value('ding_type', $element, 'field_element_type');
    if ($element_type !== FALSE && $element_type === $type) {
      $element_id = ding_base_get_value('ding_type', $element, 'field_' . $element_type);
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
 * @param $element
 *   The element you want to move.
 * @param $previous
 *   The element to position the element being moved after. If this is false
 *   it will move the element to the first position of the list.
 * @param $tell_openlist
 *   If this is true (as by default) it will tell the openlist service about
 *   the changes. When synchronizing this would be set to false.
 */
function ding_list_set_element_position(&$list, $element, $previous = FALSE, $tell_openlist = TRUE) {
  // Open a transaction, so we can rollback if ting_openlist
  $transaction = db_transaction();
  
  if (!$previous) {
    $previous = new stdClass();
    $previous->id = 0;
    $previous->openlist_id = 0;
  }
  
  
  $items = ding_base_get_items('ding_type', $list, 'field_list_objects');
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
      try {
        ding_provider_invoke('openlist', 'request', 'setElementAfter', array($element->openlist_id, $previous->openlist_id));
      } catch (Exception $e) {
        $transaction->rollback();
        drupal_set_message(t("An error occurred while sorting your elements. Please contact the administrator if this problem persists."), 'error');
        return FALSE;
      }
    }
  
    $list->field_list_objects[LANGUAGE_NONE] = $new_order;
    $list->save();
  }
}

/**
 * (Can't tell openlist about the changes yet, so don't use)
 *
 * @todo   - This function should locate all the "moves" and then call the
 *           setElementAfter() for each move, to register with openlist.
 *         - Create an openlist function, that handles full lists. Remember
 *           it should set weight and previous correct.
 */
function ding_list_set_order(&$list, $order) {
  
  $element_map = ding_list_create_dinglist_map('ding_list_element', $order);
  
  $old_order = ding_base_get_items('ding_type', $list, 'field_list_objects');
  
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
  $list_operations = ding_list_list_operations();

  // Get all the list types with autocreate.
  $auto_create = ding_list_get_list_types_by_operations(DING_LIST_OPERATION_AUTO_CREATE);
  $lists = ding_list_get_lists($auto_create, $account);
  $auto_create = array_flip($auto_create);
  
  // Run through the already created lists, and exclude them from the
  // $auto_create variable.
  if ($lists !== FALSE) {
    foreach ($lists as $list) {
      $type = ding_base_get_value('ding_type', $list, 'field_list_type');
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
    
  $save_list = FALSE;
  if (ding_user_is_provider_user($account)) {
    try {
      $creds = ding_user_get_creds($account);
      if (!empty($creds['name'])) {
        // Get the last update from the list, or set it to 0 if it doesn't exist.
        $latest = is_numeric($list->more) ? $list->more : 0;
        // Create a timestamp representing today.
        $today = mktime(0, 0, 0, date('n'), date('j'));
        
        $loans = ding_provider_invoke('historical_loans', 'get', $account, $latest);
        
        foreach ($loans as $loan) {
          if ($loan['loan_date'] < $today) {
            $save_list = TRUE;
            $ding_entity_id = variable_get('ting_agency', '') . ':' . $loan['id'];
            ding_list_create_ding_list_element($list, 'ting_object', $ding_entity_id, $account, FALSE);
            $latest = max($latest, $loan['loan_date']);
          }
        }
        /*
        // Call the alma server, and get the historical XML.
        $alma = alma_client();
        $doc = $alma->request('patron/loans/historical', array('borrCard' => $creds['name'], 'fromDate' => date('Y-m-d', $latest)));
        
        foreach ($doc->getElementsByTagName('catalogueRecord') as $item) {
          $loan_date = strtotime($item->parentNode->getAttribute('loanDate'));
          // We only add loans from yesterday.
          if ($loan_date < $today) {
            $save_list = TRUE;
            $ding_entity_id = variable_get('ting_agency', '') . ':' . $item->getAttribute('id');
            ding_list_create_ding_list_element($list, 'ting_object', $ding_entity_id, $account, FALSE);
            $latest = max($latest, $loan_date);
          }
        }
        */
      }
    }
    catch (Exception $e) {
      return t('Sorry but no list is available at this time');
    }
  }
  
  if ($save_list) {
    // Set the latest time to tomorrow.
    $latest = mktime(0, 0, 0, date('n', $latest), date('j', $latest) + 1, date('Y', $latest));
    ding_list_edit_ding_list($list, $list->title, $latest);
  }
}

/**
 * Load recent nodes tagged with $tids
 *
 * @param $tids
 *  An array of taxonomy term ids to match with nodes. The function will Load
 *  nids of nodes which are tagged with one or more of these ids.
 *  This parameter is required.
 * @param $type
 *  If provided the result will only contain nodes of this node type.
 * @param $limit
 *  If provided the result will be limited to this number.
 *
 * @return
 *  An array of nids tagged with one or more of the provided taxonomy terms.
 */
function ding_list_tids_to_nids($tids, $type = FALSE, $limit = FALSE) {

  // No tids no nids
  if (!isset($tids) || count($tids) == 0) {
    return array();
  }
  // Setup select query with the provided tids as a condition
  $query = db_select('taxonomy_index', 't')
    ->condition('tid', $tids, 'IN')
    ->addTag('node_access');
  // Join the node table
  $query->join('node', 'n', 'n.nid = t.nid');

  // If a limit has been provided add this to the query
  if ($limit !== FALSE) {
    $query->range(0, $limit);
  }

  // Published nodes only
  $query->condition('n.status', 1);

  // Optional type(=bundle) filter
  if ($type !== FALSE) {
    $query->condition('n.type', $type);
  }
  
  // Select the nid and order by the time of creation.
  $query->addField('t', 'nid');
  $query->orderBy('t.created', "DESC");
  // Return the column of nids.
  return $query->execute()->fetchCol();
}