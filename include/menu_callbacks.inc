<?php

// @todo PBJ
//   Kommenter og giv luft.

function ding_list_add_element_to_list($element_type, $list, $type_id) {
  global $user;
  if(empty($user->roles[3]) && empty($user->roles[5])) {
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
  if (!is_numeric($list)) {
    switch ($list) {
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
  }

  $element = ding_list_create_ding_list_element($list, $element_type, $type_id);
  
  $html = t('Added to !name', array('!name' => l($list->title, 'user/list/' . $list->id)));
  $status = array(
    'selector' => '#' . $context['html_id'],
    'message' => $html,
    'status' => TRUE,
    'refresh' => TRUE
  );
  $commands = array();
  // Perhaps we could remove the table row we just deleted?
  $commands[] = ajax_command_html('#' . $context['html_id'] . ' .action-message', $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}


function ding_list_remove_element_from_list($list, $element_id) {
  $context = drupal_get_query_parameters();
  ding_list_detach_ding_list_element($list, explode(',', $element_id));
  
  $html = t('Removed from !name', array('!name' => l($list->title, 'user/list/' . $list->id)));
  $status = array(
    'selector' => '#' . $context['html_id'],
    'message' => $html,
    'status' => TRUE,
    'refresh' => TRUE
  );
  $commands = array();
  // Perhaps we could remove the table row we just deleted?
  $commands[] = ajax_command_html('#' . $context['html_id'] . ' .action-message', $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

function ding_list_delete_list_callback($list_id) {
  $context = drupal_get_query_parameters();
  ding_list_delete_list($list_id);
  
  $status = array(
    'status' => TRUE,
    'remove' => '.ding-type-ding-list',
  );
  
  $html = t('List deleted');
  $commands = array();
  // Perhaps we could remove the table row we just deleted?
  $commands[] = ajax_command_append('#' . $context['html_id'], $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

function ding_list_set_order_callback($list) {
  if (!isset($_POST['order'])) {
    return drupal_json_output(array(
      'status' => FALSE
    ));
  }
  
  // ding_list_set_order($list, $_POST['order']);
  $item = entity_load_single('ding_type', $_POST['item']);
  $previous = FALSE;
  if (isset($_POST['item'])) {
    $previous = entity_load_single('ding_type', $_POST['previous']);
  }
  ding_list_set_element_position($list, $item, $previous);
  
  $html = t('List deleted');
  $commands = array();
  // Perhaps we could remove the table row we just deleted?
  $commands[] = ajax_command_changed('#' . $context['html_id'], $html, $status);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
  return drupal_json_output(array(
    'status' => TRUE
  ));
}

/**
 * Ajax callback function.
 */
function ding_list_ajax_load_content() {
  $context = drupal_get_query_parameters();
  $html_id = $context['html_id'];
  $pager_id = $context['pager_id'];
  
  $list = cache_get($context['id'], 'cache_block');
  $content = array_splice($list->data, $context['offset'], $context['limit']);
  $output = render($content);
  
  $commands = array();
  $commands[] = ajax_command_html('#' . $html_id . ' .list-content', '<div class="list-content">' . $output . '</div>');
  $commands[] = ajax_command_remove('#' . $pager_id);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Ajax callback function.
 */
function ding_list_ajax_load_tab() {
  $context = drupal_get_query_parameters();
  $html_id = $context['html_id'];
  $data = unserialize($_POST['ding_list']);
  
  unset($data['#ajax']);
  $content = render($data);
  
  $commands = array();
  $commands[] = ajax_command_replace('#' . $html_id, $content);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Ajax callback function.
 */
function ding_list_ajax_load_view() {
  $context = drupal_get_query_parameters();
  $html_id = $context['html_id'];
  $data = unserialize($_POST['ding_list']);
  
  if(!isset($data['#type']) || $data['#type'] != 'view') {
    $view_mode = $context['view_mode'];
    if($view_mode && !empty($data['#options']['context'])) {
      $data['#options']['context']['view_mode'] = $view_mode;
    }
    if($view_mode && !empty($data['#content'])) {
      foreach($data['#content'] as $index => &$obj) {
        $obj['#view_mode'] = $view_mode;
      }
    }
  }
  unset($data['#ajax']);
  $content = render($data);
  
  $commands = array();
  $commands[] = ajax_command_replace('#' . $html_id, $content);
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

function ding_list_historical_loans($account) {
  if ($account === NULL) {
    global $user;
    $account = $user;
  }
  $list = ding_list_get_list(DING_LIST_TYPE_LOAN_HISTORY, $account);
  return entity_view('ding_type', array($list), 'full');
}