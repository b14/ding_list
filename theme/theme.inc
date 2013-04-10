<?php
/**
 * @file
 * Theme and preprocess files
 *
 */

function ding_list_add_to_list($lid, $content) {
  // @todo validate input 
  $list = entity_load('ding_type', array($lid));
  entity_save('ding_type', $list);
}

function template_preprocess_ding_list_list(&$variables) {
  if ($variables['toggler'] === TRUE) {
    // $variables['toggler'] = array('#theme' => 'ding_list_mode_toggler');
  }
  
  if ($variables['sortable'] !== FALSE) {
    $variables['classes_array'][] = 'sort-enabled';
  }
  
  drupal_add_js(drupal_get_path('module', 'ding_list') . '/js/ding_list_handling.js');
}