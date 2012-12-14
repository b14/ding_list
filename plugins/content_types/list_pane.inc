<?php

$plugin = array(
  'title' => t('Ding list pane'),
  'description' => t('Display lists as a block with tabs on your page.'),
  'admin info' => 'ding_list_pane_admin_info',
  'single' => TRUE,
  'edit form' => 'ding_list_list_pane_content_type_edit_form',
  'render callback' => 'ding_list_type_render',
  'category' => t('Ding!'),
);

/**
 * Returns a summary for the administration information in panels.
 */
function ding_list_pane_admin_info($subtype, $conf, $context) {
  $output = new stdClass();
  $output->title = t('Ding list:@list', array('@list' => $conf['list']));
  $output->content = t('List: @list', array('@list' => $conf['list']));
  return $output;
}

/**
 * Implements hook_content_type_edit_form().
 */
function ding_list_list_pane_content_type_edit_form($form, &$form_state) {
  $lists = module_invoke_all('tab_lists');
  $form['list'] = array(
    '#type' => 'select',
    '#title' => t('List'),
    '#options' => $lists,
    '#default_value' => $form_state['conf']['list'],
    '#description' => t('Select the tabbed list you wish to display.'),
  );
  return $form;
}

/**
 * Implements hook_content_type_edit_form_submit().
 */
function ding_list_list_pane_content_type_edit_form_submit($form, &$form_state) {
  $form_state['conf']['list'] = $form_state['values']['list'];
}

/**
 * Render content type function.
 */
function ding_list_type_render($subtype, $conf, $args, $context) {
  $list = $conf['list'];
  module_load_include('inc', 'ding_list', 'include/block_definitions');
  $caller = format_string('ding_list_get_block_@caller', array('@caller' => $list));
  
  if (is_callable($caller)) {
    $content = $caller();
  } else {
    $content = "tst $caller";
  }
  $output['content'] = $content;
  
  $block = new stdClass();
  $block->title = '';
  $block->content = $content;
  
  return $block;
}
