<?php
/**
 * @file
 * Blocks relating to menus
 *
 */

 /**
 * Implements hook_block_info().
 */
function ding_list_block_info() {
  $blocks['search_link'] = array(
    'info' => "Link to term search result",
  );
  
  $blocks['create_ding_list'] = array(
    'info' => "Create ding list.",
  );

  return $blocks;
}

/**
 * Implements hook_block_view().
 */
function ding_list_block_view($delta) {
  
  $block = array();
  switch ($delta) {
  case 'create_ding_list':
  
    $block['content'] = array(
      '#theme' => 'ding_tabs',
      '#name' => 'your_lists',
      '#title' => t('Opret liste', array(), array('langcode' => 'da')),
      '#navigation' => 'What is navigation',
      '#more_title' => 'Læs mere her',
      '#more_link' => 'page/samtykke',
      '#mode_tabs' => array(),
      '#tabs' => array(
        'lists' => array(
          '#theme' => 'ding_tabs_content',
          '#title' => '',
          '#content' => drupal_get_form('ding_list_create_list_form'),
          '#type' => 'render',
          '#no_tab_link' => TRUE,
        ),
      ),
    );
  break;
  case 'search_link':
    if(arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2))) {
      $term = taxonomy_term_load(arg(2));
      if(!empty($term->field_search_string)) {
        $string = rawurldecode($term->field_search_string[field_language('taxonomy_term', $term, 'field_search_string')][0]['value']);
      } else {
        
        $string = rawurlencode($term->name);
      }
      global $base_path;
      $block['subject'] = '';
      $block['content'] = array(
        '#markup' => '<div class="term-link"><div class="graphic"><span class="icon"></span></div>' . 
          '<div class="content">' . 
          l('<h3 class="link-title">' . t('View search result') . '</h3>', 
            $base_path . 'search/ting/' . htmlentities($string), 
            array('html' => TRUE, 'external' => TRUE)
          ) . '</div></div>',
      );
    }
    break;
  }
  return $block;
}