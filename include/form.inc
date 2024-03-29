<?php
/**
 * @file
 * Forms.
 */

/**
 * The form used to create a ding_list object.
 *
 * If an existing ding_list object is given, that object is editted instead
 * of creating a new object.
 *
 * @param $existing
 *   This is the existing ding_list object.
 */
function ding_list_create_list_form($form, &$form_state, $existing = FALSE) {
  try {
    $consent = ding_user_consent_get_consent('loan_history_store');
  }
  catch (Exception $e) {
    $consent = FALSE;
  }
  
  if ($consent) {
    if ($existing !== FALSE) {
      $form_state['existing'] = $existing->id;
      
      $note = field_get_items('ding_type', $existing, 'field_notes');
      if ($note !== FALSE) {
        $note = $note[0]['value'];
      }
    } else {
      $form_state['existing'] = FALSE;
    }
  
    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#required' => TRUE,
      '#default_value' => isset($existing->title) ? filter_xss($existing->title) : ''
    );
    
    $form['notes'] = array(
      '#type' => 'textarea',
      '#title' => t('Note'),
      '#default_value' => isset($note) ? filter_xss($note) : ''
    );
    
    if ($existing) {
      $form['add_list'] = array(
        '#type' => 'submit',
        '#value' => t('Edit list')
      );
    }
    else {
      $form['add_list'] = array(
        '#type' => 'submit',
        '#value' => t('Add new list')
      );
    }
    
    if (isset($_GET['ting'])) {
      $form['auto_add_ting'] = array(
        '#type' => 'hidden',
        '#value' => $_GET['ting']
      );
    }
  }
  else {
    $description = variable_get('loan_history_store_description_first_time', '');
    $form['message'] = array(
      '#type' => 'item',
      '#title' => variable_get('loan_history_store_title_first_time', ''),
      '#markup' => $description['value'],
    );
  }
  return $form;
}
/**
 * The create_list_form submit handler.
 */
function ding_list_create_list_form_submit($form, &$form_state) {
  // Create the new list object.
  if (!$form_state['existing']) {
    $list = ding_list_create_ding_list(
      $form_state['values']['title'],
      DING_LIST_TYPE_USER_LIST,
      NULL,
      NULL,
      FALSE,
      array(
        array(
          'name' => 'field_notes',
          'value_type' => 'value',
          'value' => $form_state['values']['notes'],
        )
      )
    );
    
    if ($list === FALSE) {
      $form_state['redirect'] = 'user/manage-lists';
      return;
    }
  }
  else {
    $list = ding_list_get_list($form_state['existing']);
    
    ding_list_edit_ding_list(
      $list,
      $form_state['values']['title'],
      NULL,
      TRUE,
      array(
        array(
          'name' => 'field_notes',
          'value_type' => 'value',
          'value' => $form_state['values']['notes'],
        )
      )
    );
  }
  
  if (isset($form_state['values']['auto_add_ting'])) {
    ding_list_create_ding_list_element($list, 'ting_object', $form_state['values']['auto_add_ting']);
  }
  
  $form_state['redirect'] = 'user/list/' . $list->id;
}