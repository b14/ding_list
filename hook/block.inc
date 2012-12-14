<?php
/**
 * @file
 * Blocks relating to menus
 *
 */

function ding_list_block_info() {
  $lists = ding_list_get_blocks();
  $blocks = array();
  foreach ($lists as $list => $info) {
    $blocks[$list] = array(
      'info' => "Ding list:$list",
      'cache' => DRUPAL_NO_CACHE,
    );
  }
  $blocks['search_link'] = array(
    'info' => "Link to term search result",
  );
  
  $blocks['create_ding_list'] = array(
    'info' => "Create ding list.",
  );

  return $blocks;
}

function ding_list_block_view($delta) {
  
  $block = array();
  switch ($delta) {
  case 'create_ding_list':
    $block['content'] = drupal_get_form('ding_list_create_list_form');
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
  default:
    // Manual block hide via taxonomy/term/$tid/edit
    if (main_subterms_is_block_hidden($delta)) {
      return array();
    }
    
    module_load_include('inc', 'ding_list', 'include/block_definitions');
    $caller = "ding_list_get_block_$delta";
    if (is_callable($caller)) {
      $content = $caller();
      $block['subject'] = "";
      $block['content'] = $content;
    }
    else {
      $block['content'] = "tst $caller";      
    }  
  }
  return $block;
}

function ding_list_get_blocks() {
  return array(
    'inspiration' => array(),
    'calendar' => array(),
    'blog' => array(),
    'news' => array(),
    'your_lists' => array(),
    'your_activities' => array(),
    'ding_list' => array(),
    'loan_list' => array(),
    'reserved_list' => array(),
    'debt_list' => array(),
    'user_edit' => array(),
    'related_pages_list' => array()
  );
}

function ding_list_tab_lists() {
  return array(
    'inspiration' => t('Inspiration'),
    'calendar' => t('Calender'),
    'blog' => t('Blog'),
    'news' => t('News'),
    'your_lists' => t('Your lists'),
    'your_activities' => t('Your activities'),
    'ding_list' => t('Ding list'),
    'loan_list' => t('Loan list'),
    'reserved_list' => t('Reserved list'),
    'debt_list' => t('My debts'),
    'user_edit' => t('Edit user'),
  );
}