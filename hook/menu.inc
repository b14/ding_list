<?php
/**
 * @file
 * Menu
 *
 */
 
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
  if (!empty($list)) {
    return ($user->uid === $list->uid);
  }
  else {
    return TRUE;
  }
}