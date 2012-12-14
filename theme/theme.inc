<?php
/**
 * @file
 * Theme and preprocess files
 *
 */

function ding_list_add_to_list($lid,$content) {
  // @todo validate input 
  $list = entity_load('ding_type',array($lid));
  entity_save('ding_type',$list);
}

function ding_list_preprocess_node(&$variables) {

}

function template_preprocess_ding_list_test(&$variables) {

}


function template_preprocess_ding_list(&$variables) {
  global $user;
    
  if (isset($variables['more_link'])) {
    $variables['more'] = l("<span class=\"icon\"></span>" . $variables['more_title'], $variables['more_link'], array('html' => true, 'attributes' => array('class' => array('more'))));
  } else {
    $variables['more'] = "";
  }
  
  $variables['classes_array'][] = "ding-list-" . $variables['name'];

  // Navigation
  $html_id = drupal_html_id('ding-list-content');
  $list_id = drupal_html_id('ding-list');
  
  $variables['list_attributes']['id'] = $list_id;
  
  // Create tab anchor navigation  
  $tabs_links = array();
  $cnt = 0;
  $first_id = NULL;
  foreach ($variables['tabs'] as $index => $tab) {
    // Skip empty tabs
    if (isset($variables['tabs'][$index]['#content']) && is_array($variables['tabs'][$index]['#content']) && count($variables['tabs'][$index]['#content']) == 0) {
      unset($variables['tabs'][$index]);
      continue;
    }
    $anchor = $variables['name'] . "-$index";
    $tab_id = drupal_html_id($anchor);
    $content_id = $tab_id . '-content';
    if(empty($first_id))
      $first_id = $content_id;
    $variables['tabs'][$index]['#divid'] = $content_id;
  	// Save the array definition to javascript to enable view mode switching.
  	$vars = $variables['tabs'][$index];
  	if(!empty($vars['#options']['context'])) {
  	  $vars['#options']['context']['path'] = (drupal_is_front_page()) ? 'taxonomy/term/5' : current_path();
  	}
  	drupal_add_js(array('ding_list' => array($tab_id => serialize($vars))), 'setting');
	
    if(isset($variables['tabs'][$index]['#ajax']) && $variables['tabs'][$index]['#ajax'] == TRUE) {
      $options = array(
        'attributes' => array(
          'id' => $tab_id,
          'class' => array('use-ajax'),
        ),
        'query' => array(
          'html_id' => $content_id,
        ),
      );
      if(isset($_GET['debug']) && $_GET['debug'] == 's') {
        $options['query']['debug'] = 's';
      }
      $tabs_links[] = l($tab['#title'], 'ding/list/tab', $options);
      //$variables['tabs'][$index]['#options']['html_id'] = $html_id;
      
      $html_id = drupal_html_id('ding-list-content');
    } else {
      $options = array(
        'attributes' => array(
          'id' => $tab_id,
        ),
        'fragment' => $anchor,
        'external' => TRUE,
      );
      $tabs_links[] = l($tab['#title'], '', $options);
    }
    $variables['tabs'][$index]['#cnt'] = $cnt++;
  }
  if(count($variables['tabs']) <= 1) {
    $variables['classes_array'][] = 'tabs-hidden';
  }
  $variables['navigation'] = theme('ding_list_view_tabs', array('html_id' => $first_id, 'tabs' => $variables['mode_tabs']));
  //
 
  if (!isset($tab['#no_tab_link']) || $tab['#no_tab_link'] !== TRUE) {
    $variables['tabs']['links'] = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#items' => $tabs_links,
    );
  }
  
  if ($cnt === 0) {
    unset($variables['tabs']);
  }
}

function template_preprocess_ding_list_view_tabs(&$variables) {
  $html_id = $variables['html_id'];
  
  $items = array();
  $options = array(
    'attributes' => array(),
    'query' => array(
      'html_id' => $html_id,
    ),
  );
  if(isset($_GET['debug']) && $_GET['debug'] == 's') {
    $options['query']['debug'] = 's';
  }
  
  foreach ($variables['tabs'] as $tab) {
    $options['attributes']['id'] = drupal_html_id('view-tabs');
    $options['attributes']['class'] = array(
      'use-ajax',
      'change-view',
      drupal_html_class('toggle-' . $tab[1]),
    );
    $options['query']['view_mode'] = $tab[1];
    $items[] = l($tab[0], 'ding/list/view', $options);
  }
  
  $variables['navigation'] = array(
    '#theme' => 'item_list',
    '#type' => 'ul',
    '#items' => $items,
  );
}

function theme_ding_list_view_tabs($variables) {
  return render($variables['navigation']);
}

function template_preprocess_ding_list_content(&$variables) {
  
  // First tab is visible, the rest are hidden
  if ($variables['cnt'] != 0) {
    $variables['classes_array'][] = "tab-hidden";
  } 
  if(isset($variables['ajax']) && $variables['ajax']) {
    //$html_id = $variables['#divid'];
    $variables['prefix'] = '<div class="ding-list-ajax-load">';
    $variables['content'] = '';
    $variables['suffix'] = '</div>';
  } else {
    switch ($variables['type']) {
      case 'ting_list_collection':
        $context = drupal_get_query_parameters();
        $view_mode = 'list_item';
        if (isset($context['view_mode'])) {
          $view_mode = $context['view_mode'];
        }
        
        $collection = ting_collection_load($variables['options']['object_id']);
        $list = array();
        if(!empty($collection)) {
          foreach ($collection->getEntities() as $entity) {
            if ($entity->id != $variables['options']['object_id']) {
              $list[] = ting_object_view($entity, $view_mode);
            }
          }
        }
        if (count($list) > 0) {
          $variables['content'] = render($list);
        }
        else {
          $variables['no_result'] = array(
            array(
              'key_val' => array('hide', '.pagination'),
              'condition' => array('not_exist', '.tabs')
            )
          );
        }
        
        return;
        
      break;
      
      case 'user_list':
        $context = drupal_get_query_parameters();
        $view_mode = 'list_item';
        if (isset($context['view_mode'])) {
          $view_mode = $context['view_mode'];
        }
        
        $ding_list = ding_list_get_list($variables['list_id']);
        $items = field_get_items('ding_type', $ding_list, 'field_list_objects');
        if ($items !== FALSE) {
          drupal_add_js(drupal_get_path('theme', 'fkb') . '/js/ding_list_handling.js');
          $ids = array();
          foreach ($items as $item) {
            $ids[] = $item['target_id'];
          }
          
          $items = entity_load('ding_type', $ids);
          foreach ($items as &$item) {
            $item->parent_list = $ding_list->id;
            $item->item_view_mode = $view_mode;
          }
          
          $list = array(
            '#theme' => 'ding_list_list',
            '#items' => entity_view('ding_type', $items, 'teaser')
          );
          if (ding_list_allowed($ding_list, DING_LIST_OPERATION_SORT)) {
            $list['#sortable'] = $ding_list->id;
          }
          $variables['content'] = render($list);
        }
      break;
    
      case 'view':
        if(isset($variables['view_display'])) {
          $context = drupal_get_query_parameters();          
          if(is_array($variables['view_display'])) {
            $display = (!empty($context['view_mode'])) ? $variables['view_display'][$context['view_mode']] : array_shift($variables['view_display']);
          } else {
            $display = $variables['view_display'];
          }
        } else {
          $display = "default";
        }
        
        // If we simply skip the arguments it will automatically select the default argument.
        $args = NULL;
        if (isset($variables['view_args'])) {
          $args = $variables['view_args'];
        }
        
        $view = views_get_view($variables['view_name']);
        if(isset($context['view_all'])) {
          $view->set_items_per_page(40);
        }
        if (!$view || !$view->access($display)) {
          $variables['content'] = array();
        } else {
          if(is_array($args)) {
            $args = array_values($args);
          }
          $variables['content'] = $view->preview($display, $args);
        }
        if(!isset($context['view_all']) && $variables['link'] !== FALSE) {
          $data = array(
            '#theme' => 'ding_list_content',
            '#title' => $variables['title'],
            '#type' => 'view',
            '#view_name' => $variables['view_name'],
            '#view_display' => $display,
            '#view_args' => $args,
          );
          $html_id = drupal_html_id('ding-list');
          $variables['prefix'] = '<div class="ding-list-ajax-load" id="' . $html_id . '">';
          $link_id = drupal_html_id('view-more');
          $variables['suffix'] = '</div>';
          $options = array(
            'attributes' => array(
              'id' => $link_id,
              'class' => array(
                'use-ajax',
                'view-more',
              ),
            ),
            'query' => array(
              'html_id' => $html_id,
              'view_all' => '1',
              'view_mode' => 'grid_item',
            ),
            'html' => true,
          );
          drupal_add_js(array('ding_list' => array($link_id => serialize($data))), 'setting');
          //$variables['#js_data'] = array('ding_list' => array($link_id => serialize($data)));
          $variables['pager'] = '<div class="pager">' . l('<span class="icon"></span>' . t('View more'), 'ding/list/view', $options) . '</div>';
        }
        /*$view_args = array(
          $variables['view_name'],
          $display
        );
        $view_args = array_merge($view_args, $args);
        $variables['content'] = call_user_func_array('views_embed_view', $view_args);*/
        // $variables['content'] = views_embed_view($variables['view_name'], $display, $args);
        break;
      case 'serendipity':
        $options = $variables['options'];
        if(!isset($options['options']))
          $options['options'] = array();
        $content = ding_serendipity_render($options['context'], $options['options']);
        if (!empty($variables['limit']) && count($content) > $variables['limit']) {
          global $user;
          if(isset($variables['options'])) {
            $html_id = isset($variables['#divid']) ? $variables['#divid'] : drupal_html_id('ding-list');
            $path = !empty($vars['#options']['context']['path']) ? $vars['#options']['context']['path'] : current_path();
          } else {
            $html_id = drupal_html_id('ding-list');
            $path = current_path();
          }
          
          $content_id = str_replace('-', '_', $html_id . '-' . $user->uid);
  
          // Set cache
          $expire = 600;
          $cache_id = str_replace('_', ':', $content_id) . ':' . $path;
          cache_set($cache_id, $content, 'cache_block', REQUEST_TIME + $expire);
          $variables['prefix'] = '<div class="ding-list-ajax-load" id="' . $html_id . '">';
          $variables['suffix'] = '</div>';
          $variables['pager'] = array(
            '#theme' => 'ding_list_pager',
            '#query' => array(
              'html_id' => $html_id,
              'id' => $cache_id,
              'limit' => $variables['limit'],
            ),
            '#id' => $content_id,
            '#length' => count($content),
            '#limit' => $variables['limit'],
          );
          $variables['content'] = render(array_splice($content, 0, $variables['limit']));
        } else {
          $variables['content'] = render($content);
        }
        break;
      case 'render':
        $context = drupal_get_query_parameters();
        if (!empty($variables['limit']) && count($variables['content']) > $variables['limit']) {
          global $user;
          if(isset($variables['options']))
            $html_id = isset($variables['#divid']) ? $variables['#divid'] : drupal_html_id('ding-list');
          else
            $html_id = drupal_html_id('ding-list');
          $path = current_path();
          $content_id = str_replace('-', '_', $html_id . '-' . $user->uid);
  
          // Set cache
          $expire = 600;
          $cache_id = str_replace('_', ':', $content_id) . ':' . $path;
          cache_set($cache_id, $variables['content'], 'cache_block', REQUEST_TIME + $expire);
          
          $variables['prefix'] = '<div class="ding-list-ajax-load" id="' . $html_id . '">';
          $variables['suffix'] = '</div>';
          $variables['pager'] = array(
            '#theme' => 'ding_list_pager',
            '#query' => array(
              'html_id' => $html_id,
              'id' => $cache_id,
              'limit' => $variables['limit'],
            ),
            '#id' => $content_id,
            '#length' => count($variables['content']),
            '#limit' => $variables['limit'],
          );
          $variables['content'] = render(array_splice($variables['content'], 0, $variables['limit']));
        } else {
          $variables['content'] = render($variables['content']);
        }
        break;
    }
    if(empty($variables['content'])) {
      $html_id = drupal_html_id('ding-list');
      $variables['prefix'] = '<div class="ding-list-ajax-load" id="' . $html_id . '">';
      $variables['suffix'] = '</div>';
      $variables['content'] = '<div class="content">' . t('Unfortunetly no content was found.') . '</div>';
    }
  }
}

/**
 * Theme function for ding_list_pager
 */
function theme_ding_list_pager($variables) {
  drupal_add_js(drupal_get_path('module', 'ding_list') . '/js/ding_list.js');
  $id = $variables['id'];
  $length = $variables['length'];
  $limit = $variables['limit'];
  $query = $variables['query'];
  
  $pager_id = drupal_html_id('ding-list-pager');
  
  $output = '<div class="pager" id="' . $pager_id . '">';
  if(!empty($_GET['debug']) && $_GET['debug'] == 's') {
    $query['debug'] = 's';
  }
  $query['pager_id'] = $pager_id;
  $query['offset'] = 0;
  $query['limit'] = $variables['length'];
  $output .= l(
      '<span class="icon"></span>' . t('View more'), 
      'ding/list/fetch', 
      array(
        'html' => true,
        'attributes' => array(
          'id' => drupal_html_id('ding-list-pager-all'),
          'class' => array('use-ajax'),
        ),
        'query' => $query,
      )
    );
  $output .= '</div>';
  return $output;
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
  ),'setting');
  
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
}

/**
 * Implements hook_theme().
 *
 */
function ding_list_theme() {
  return array(
    'ding_list'  => array(
      'template' => 'theme/ding_list',
      'variables' => array(
        'name' => "",
        'title' => "List title",
        'navigation' => "My navigation",
        'tabs' => array(),
        'context' => array('front'),
        'more_link' => NULL,
        'more_title' => "Læs mere",
        'mode_tabs' => array(
          array(t('Expanded list'), 'grid_item'),
          array(t('Overview list'), 'list_item'),
          array(t('Compact list'), 'list_item_thin'),
        ),
      ),
    ),
    'ding_list_header' => array(
      'template' => 'theme/ding-list-header',
      'variables' => array(
        'title' => ''
      )
    ),
    'ding_list_content'  => array(
      'template' => 'theme/ding_list_content',
      'variables' => array(
        'title' => 'Inspiration til dig',
        'type' => 'array',
        'name' => '',
        'link' => 'blabla',
        'content' => 'Min Inspiration',
        'ajax' => FALSE,
        'context' => array('front'),
        'view_name' => '',
        'view_display' => NULL,
        'view_args' => NULL,
        'divid' => '',
        'cnt' => '',
        'limit' => NULL,
        'options' => NULL,
        'list_id' => NULL,
      ),
    ),
    'ding_list_test'  => array(
      'template' => 'theme/ding_list_test',
      'variables' => array(
        'from_preprocess' => NULL,
      ),
    ),
    'ding_list_pager' => array(
      'variables' => array(
        'id' => 0, 
        'query' => '',
        'length' => 0, 
        'limit' => 0
      ),
    ),
    'ding_list_view_tabs' => array(
      'variables' => array(
        'html_id' => '',
        'tabs' => array()
      )
    ),
    'ding_list_mode_toggler' => array(
      'variables' => array(
        'modes' => array(t('Compact'), t('Expanded')),
        'default' => 'expanded'
      )
    ),
    'ding_list_list'  => array(
      'template' => 'theme/ding-list-list',
      'variables' => array(
        'items' => array(),
        'sortable' => FALSE,
        'toggler' => TRUE
      ),
    ),
  );
}
