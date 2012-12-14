<?php
/**
 * Implements hook_menu().
 */
function ding_list_menu() {
  $items = array();
  
  // A simple base for all menu calls related to editing a list.
  $edit_list_base = array(
    'access arguments' => array(2),
    'access callback' => 'ding_list_user_has_list_edit_access',
    'file' => 'include/menu_callbacks.inc',
    'type' => MENU_CALLBACK,
  );

  // Attach a ding_list_element to a list.
  // We're manually writing all the element_types here manually, because with
  // the few types we have it's faster than reading them from the field
  // definition.
  // The 3rd argument is the list id, the 4th is the id (or string).
  $items['dinglist/attach/ting_object/%/%'] =
  $items['dinglist/attach/taxonomy_term/%/%'] = array_merge($edit_list_base, array(
    'title' => 'Main test',
    'page callback' => 'ding_list_add_element_to_list',
    'page arguments' => array(2, 3, 4),
    'delivery callback' => 'ajax_deliver',
    'access arguments' => array(3)
  ));
  
  // Detach an element from a list.
  // Only the list id and actual element->id is needed.
  $items['dinglist/detach/%/%'] = array_merge($edit_list_base, array(
    'title' => 'Main test',
    'page callback' => 'ding_list_remove_element_from_list',
    'page arguments' => array(2, 3),
    'delivery callback' => 'ajax_deliver',
  ));
  
  // Delete a list.
  $items['dinglist/delete/%'] = array_merge($edit_list_base, array(
    'title' => 'Main test',
    'page callback' => 'ding_list_delete_list_callback',
    'page arguments' => array(2),
    'delivery callback' => 'ajax_deliver',
  ));
  
  // Set the order of a list.
  // It gets the ordered array as a POST through ajax.
  $items['dinglist/set_order/%'] = array_merge($edit_list_base, array(
    'title' => 'Main test',
    'page callback' => 'ding_list_set_order_callback',
    'page arguments' => array(2),
    'delivery callback' => 'ajax_deliver',
  ));
  
  // Ajax callback urls
  $items['ding/list/fetch'] = array(
    'title' => 'List ajax callback',
    'page callback' => 'ding_list_ajax_load_content',
    'delivery callback' => 'ajax_deliver',
    'access arguments' => array('access content'),
    'file' => 'include/menu_callbacks.inc',
    'type' => MENU_CALLBACK,
  );
  $items['ding/list/tab'] = array(
    'title' => 'List ajax callback',
    'page callback' => 'ding_list_ajax_load_tab',
    'delivery callback' => 'ajax_deliver',
    'access arguments' => array('access content'),
    'file' => 'include/menu_callbacks.inc',
    'type' => MENU_CALLBACK,
  );
  $items['ding/list/view'] = array(
    'title' => 'List ajax callback',
    'page callback' => 'ding_list_ajax_load_view',
    'delivery callback' => 'ajax_deliver',
    'access arguments' => array('access content'),
    'file' => 'include/menu_callbacks.inc',
    'type' => MENU_CALLBACK,
  );
  
  $items['dinglist/from-local-to-openlist'] = array(
    'title' => 'Temporary.',
    'page callback' => 'from_local_to_openlist',
    'access arguments' => array('access content')
  );
  
  /*
  $items['user/%user/historical-loans'] = array(
    'title' => 'Historical loans',
    'page callback' => 'ding_list_historical_loans',
    'page arguments' => array(1),
    'access callback' => 'user_view_access',
    'access arguments' => array(1),
    'file' => 'include/menu_callbacks.inc',
  );
  
  $items['user/history'] = array(
    'title' => 'Historical loans',
    'page callback' => 'ding_list_historical_loans',
    'page arguments' => array(NULL),
    'access arguments' => array('access content'),
    'file' => 'include/menu_callbacks.inc',
  );
  */
  
  return $items;
}

/**
 * Check if the current user has edit permissions to the list id.
 */
function ding_list_user_has_list_edit_access($list_id) {
  global $user;
  
  if (!is_numeric($list_id)) {
    return TRUE;
  }
  
  $list = entity_load_single('ding_type', $list_id);
  if(!empty($list))
    return ($user->uid === $list->uid);
  else
    return TRUE;
}


// TEMPORARY FUNCTION.
function from_local_to_openlist() {

  $page = 0;
  $range = 150;
  
  if (isset($_GET['page'])) {
    $page = $_GET['page'];
  }
  if (isset($_GET['range'])) {
    $range = $_GET['range'];
  }
  
  $offset = $page * $range;
  
  $start = microtime(TRUE);
  
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'ding_type')
    ->entityCondition('bundle', 'ding_list')
    ->propertyOrderBy('id', 'ASC')
    ->range($offset, $range);
  
  $result = $query->execute();
  if (!empty($result)) {
    $lists = entity_load('ding_type', array_keys($result['ding_type']));
  }
  
  foreach ($lists as $list) {
    $users = array();
    if (!isset($users[$list->uid])) {
      $account = user_load($list->uid);
      $users[$list->uid] = $account->name;
    }
    
    // Create the list.
    $list_type = b14_tools_get_value('ding_type', $list, 'field_list_type');
    // $list->openlist_id = ting_openlist_request('createList', array($account->name, $list->title, $list_type, $list->more));
    // $list->save();
    b14dpm(5, $list);
    
    $items = ding_list_load_elements($list);
    foreach ($items as $item) {
      $element_type = b14_tools_get_value('ding_type', $item, 'field_element_type');
      $element_id = b14_tools_get_value('ding_type', $item, 'field_' . $element_type);
      
      // $item->openlist_id = ting_openlist_request('createElement', array($list->openlist_id, array('id' => $element_id, 'type' => $element_type)));
      // $item->save();
      b14dpm(5, $item);
    }
  }
  
  return 'TIME: ' . (microtime(TRUE) - $start);
}