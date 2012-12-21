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

/*
 * Theme ding list mode toggler
 *  Simply themes a mode toggler list.
 *  as an default the choices are 'compact' / 'expanded'
 *
 *  The corresponding js can be found in this module 'js/ding_list.js'
 */
function theme_ding_list_mode_toggler($variables) {
  drupal_add_js(array(
    'dingList' => array(
      'listModes' => $variables['modes'],
      'listTypeDefault' => $variables['default'],
    )
  ), 'setting');
  
  $links = array();
  foreach ($variables['modes'] as $mode) {
    $links[] = array(
      'title' => $mode,
      'href' => $_GET['q'], // Simple linking to the current page..
      'html' => 'true',
      'attributes' => array(
        'id' => $mode
      )
    );
  }
  // Render arrays seems to be broken with theme_links (?)  
  return '<div class="ding-list-mode-toggler">' . theme('links', array(
    'links' => $links,
    'attributes' => array('class' => array('links', 'inline'))
  )) . '</div>';
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