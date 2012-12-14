<?php

/**
 * Create an array that maps a list of openlist ids to the local entity
 * objects.
 *
 * @param $type
 *   The ding_type bundle.
 * @param $openlist_ids
 *   An array with the openlist ids.
 *
 * @return
 *   The array map with the following structure:
 *   arr[openlist_id] = entity
 */
function ding_list_create_openlist_map($type, $openlist_ids) {
  if (empty($openlist_ids)) {
    return array();
  }
  
  $openlist_ids = array_unique($openlist_ids);
  
  $query = new EntityFieldQuery();
  $result = $query->entityCondition('entity_type', 'ding_type')
    ->entityCondition('bundle', $type)
    ->propertyCondition('openlist_id', $openlist_ids)
    ->execute();
    
  $map = array();
  if (!empty($result)) {
    $locals = entity_load('ding_type', array_keys($result['ding_type']));
    
    foreach ($locals as $local) {
      $map[$local->openlist_id] = $local;
    }
  }
  return $map;
}

/**
 * Create an array that maps a list of local ids to the local entity objects.
 *
 * @param $type
 *   The ding_type bundle.
 * @param $dinglist_ids
 *   An array with the local ids.
 *
 * @return
 *   The array map with the following structure:
 *   arr[local_id] = entity
 */
function ding_list_create_dinglist_map($type, $dinglist_ids) {
  if (empty($dinglist_ids)) {
    return array();
  }
  
  $dinglist_ids = array_unique($dinglist_ids);
  
  $query = new EntityFieldQuery();
  $result = $query->entityCondition('entity_type', 'ding_type')
    ->entityCondition('bundle', $type)
    ->propertyCondition('id', $dinglist_ids)
    ->execute();
    
  $map = array();
  if (!empty($result)) {
    $locals = entity_load('ding_type', array_keys($result['ding_type']));
    
    foreach ($locals as $local) {
      $map[$local->id] = $local;
    }
  }
  return $map;
}


/**
 * Update the local database, with the changes from openlist.
 *
 * @param $account
 *   Which accoun to synchronize (defaults to the logged in user).
 */
function ding_list_sync_openlist($account = NULL) {
  if ($account === NULL) {
    global $user;
    $account = $user;
  }

  //
  // Make sure the user is a library user.
  // $creds = ding_user_get_creds($account);
  // if (!$creds) {
    // return;
  // }
  
  //
  // Get the last modified timestamp.
  $from = $user->openlist_modified;
  
  //
  // Fetch the modified lists and elements.
  $openlist_lists = ting_openlist_request('getLists', array($account->name, $from));
  $openlist_elements = ting_openlist_request('getUserElements', array($account->name, $from));
  
  //
  // Create a 2 arrays that map the openlist ids to local ids.
  // One array for lists and one for elements.
  // While running through the lists update the $from so it got the latest
  // modified list or element.
  $list_map = $element_map = array();
  foreach ($openlist_lists as $list) {
    $list_map[] = $list['list_id'];
    $from = max($list['modified'], $from);
  }
  foreach ($openlist_elements as $element) {
    $list_map[] = $element['list_id'];
    $element_map[] = $element['element_id'];
    if ($element['previous'] > 0) {
      $element_map[] = $element['previous'];
    }
    $from = max($element['modified'], $from);
  }
  $list_map = ding_list_create_openlist_map('ding_list', $list_map);
  $element_map = ding_list_create_openlist_map('ding_list_element', $element_map);
  
  //
  // If needed update the openlist_modified variable on the user.
  if ($from > $user->openlist_modified) {
    $user->openlist_modified = $from;
    user_save($user);
  }
  
  //
  // Run through all the modified lists.
  foreach ($openlist_lists as $openlist_list) {
    // The list already exists, so we need to either edit or delete it.
    if (isset($list_map[$openlist_list['list_id']])) {
      $local_list = &$list_map[$openlist_list['list_id']];
      
      // We only edit the list if its status is 1
      if ($openlist_list['status'] == 1) {
        // b14dpm(3, 'List edit: ' . $openlist_list['list_id']);
        if (!isset($openlist_list['data']['fields']) || !is_array($openlist_list['data']['fields'])) {
          $openlist_list['data']['fields'] = array();
        }
        ding_list_edit_ding_list($local_list, $openlist_list['title'], $openlist_list['data']['more'], FALSE, $openlist_list['data']['fields']);
      }
      // Remove it locally
      else {
        // b14dpm(3, 'List delete: ' . $openlist_list['list_id']);
        ding_list_delete_list($local_list, FALSE);
      }
    }
    // The list is not present in the local database, so we need to create it.
    else {
      // We only create the list, if it has the status of 1
      if ($openlist_list['status'] == 1) {
        // b14dpm(3, 'List create: ' . $openlist_list['list_id']);
        if (!isset($openlist_list['data']['fields']) || !is_array($openlist_list['data']['fields'])) {
          $openlist_list['data']['fields'] = array();
        }
        $list_map[$openlist_list['list_id']] = ding_list_create_ding_list(
          $openlist_list['title'],
          $openlist_list['type'],
          $account,
          $openlist_list['data']['more'],
          $openlist_list['list_id'],
          $openlist_list['data']['fields']
        );
      }
    }
  }
  
  //
  // Run through all the modified elements.
  $save_list = array();
  foreach ($openlist_elements as $openlist_element) {
    // The element already exists, so we need to either edit or delete it.
    if (isset($element_map[$openlist_element['element_id']])) {
      
      // We only edit the list if its status is 1
      if ($openlist_element['status'] == 1) {
        // b14dpm(3, 'Element edit: ' . $openlist_element['element_id']);
        
        $local_element = $element_map[$openlist_element['element_id']];
        $local_list = &$list_map[$openlist_element['list_id']];
        
        ding_list_edit_ding_list_element(
          $local_element,
          $openlist_element['data']['type'],
          $openlist_element['data']['id'],
          FALSE
        );
        
        if ($openlist_element['previous'] > 0) {
          $previous_element = $element_map[$openlist_element['previous']];
        } else {
          $previous_element = new stdClass();
          $previous_element->id = 0;
        }
        
        
        $items = b14_tools_get_items('ding_type', $local_list, 'field_list_objects');
        $new_order = array();
        foreach ($items as $key => $item) {
          if ($item['target_id'] !== $local_element->id) {
            $new_order[] = $item;
          }
          
          if ($item['target_id'] === $previous_element->id) {
            $new_order[] = array('target_id' => $local_element->id);
          }
        }
        if ($openlist_element['previous'] == 0) {
          array_unshift($new_order, array('target_id' => $local_element->id));
        }
        
        if ($new_order != $items) {
          $local_list->field_list_objects[LANGUAGE_NONE] = $new_order;
          $save_list[$openlist_element['list_id']] = $openlist_element['modified'];
        }
      }
      // Remove it locally
      else {
        // b14dpm(3, 'Element delete: ' . $openlist_element['element_id']);
        // Update the save_list, so we can save all the lists changed
        $save_list[$openlist_element['list_id']] = $openlist_element['modified'];
        
        ding_list_detach_ding_list_element(
          $list_map[$openlist_element['list_id']],
          array($element_map[$openlist_element['element_id']]->id),
          FALSE, // save_list
          FALSE // no_openlist
        );
      }
    }
    // The list is not present in the local database, so we need to create it.
    else {
      // We only create the list, if it has the status of 1
      if ($openlist_element['status'] == 1) {
        // b14dpm(3, 'Element create: ' . $openlist_element['element_id']);
        // Update the save_list, so we can save all the lists changed
        $save_list[$openlist_element['list_id']] = $openlist_element['modified'];
        
        $element = ding_list_create_ding_list_element(
          $list_map[$openlist_element['list_id']],
          $openlist_element['data']['type'],
          $openlist_element['data']['id'],
          $account,
          FALSE, // save_list
          $openlist_element['element_id']
        );
        $element_map[$element->openlist_id] = $element;
      }
    }
  }
  // Run through the save_list, update their modified and save them.
  foreach ($save_list as $openlist_id => $modified) {
    $list = $list_map[$openlist_id]; 
    // $list->openlist_modified = max($modified, $list->openlist_modified);
    $list->save();
  }
}
