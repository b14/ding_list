<?php
/**
 * @file
 * Container for menu callback functions.
 */

/**
 * Add an element to a list.
 */
function ding_list_add_element_to_list($element_type, $list_id, $type_id) {
  global $user;
  if (module_exists('ding_user_consent') && (empty($user->roles[3]) && empty($user->roles[5]))) {
    try {
      $consent = ding_user_consent_get_consent('loan_history_store');
    }
    catch (Exception $e) {
      return array(
        '#type' => 'ajax',
        '#commands' => array(
          ajax_command_ding_user_consent(array())
        ),
      );
    }
  }
  $context = drupal_get_query_parameters();
  if (!is_numeric($list_id)) {
    switch ($list_id) {
      case DING_LIST_TYPE_FOLLOW:
        $lists = ding_list_get_lists(DING_LIST_TYPE_FOLLOW, NULL, array(0, 1));
        if ($lists !== FALSE) {
          $list = current($lists);
        }
        else {
          $list = ding_list_create_ding_list('Follow', DING_LIST_TYPE_FOLLOW);
        }
      break;
    }
  }  else {
    $list = ding_list_get_list($list_id);
  }

  $element = ding_list_create_ding_list_element($list, $element_type, $type_id);
  if ($element !== FALSE) {
    $html = t('Added to !name', array('!name' => l($list->title, 'user/list/' . $list->id)));
  } else {
    $html = t("An error occurred, your element wasn't created. Please contact the administrator if this problem persists.");
  }
  $status = array(
    'selector' => '#' . $context['html_id'],
    'message' => $html,
    'status' => TRUE,
    'refresh' => TRUE
  );
  $commands = array();
  
  $commands[] = ajax_command_html('#' . $context['html_id'] . ' .action-message', $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Remove an element from a list.
 */
function ding_list_remove_element_from_list($list_id, $element_id) {
  $list = ding_list_get_list($list_id);
  $context = drupal_get_query_parameters();
  if (ding_list_detach_ding_list_element($list, explode(',', $element_id)) !== FALSE) {
    $html = t('Removed from !name', array('!name' => l($list->title, 'user/list/' . $list->id)));
  } else {
    $html = t("An error occurred while removing your element. Please contact the administrator if this problem persists.");
  }
  
  $status = array(
    'selector' => '#' . $context['html_id'],
    'message' => $html,
    'status' => TRUE,
    'refresh' => TRUE
  );
  $commands = array();
  
  $commands[] = ajax_command_html('#' . $context['html_id'] . ' .action-message', $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Delete a list.
 */
function ding_list_delete_list_callback($list_id) {
  $list = ding_list_get_list($list_id);
  
  $context = drupal_get_query_parameters();
  ding_list_delete_list($list);
  
  $status = array(
    'status' => TRUE,
    'remove' => '.ding-type-ding-list',
  );
  
  $html = t('List deleted');
  $commands = array();
  
  $commands[] = ajax_command_append('#' . $context['html_id'], $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Reorder a list.
 */
function ding_list_set_order_callback($list_id) {
  if (!isset($_POST['order'])) {
    return drupal_json_output(array(
      'status' => FALSE
    ));
  }
  
  $list = ding_list_get_list($list_id);
  
  // We're not using: ding_list_set_order($list, $_POST['order']);
  // Because it's not setup to work properly with openlist yet.
  
  $item = entity_load_single('ding_type', $_POST['item']);
  $previous = FALSE;
  if (isset($_POST['item'])) {
    $previous = entity_load_single('ding_type', $_POST['previous']);
  }
  ding_list_set_element_position($list, $item, $previous);
  
  $html = t('List deleted');
  $commands = array();
  
  $commands[] = ajax_command_changed('#' . $context['html_id'], $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
  return drupal_json_output(array(
    'status' => TRUE
  ));
}