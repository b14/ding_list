<?php

// $id$

/**
 * @file
 * empty module which is used for custom development.
 *
 * Description of the module.
 */

function ding_list_get_example_teasers() {

  return render(ding_serendipity_render(
    array(
      'frontpage', 
      'field_lists',
      'view_mode' => 'big_teaser',
      'random' => TRUE
    )
  ));
}

function ding_list_get_example_teasers2() {
  $mult = ding_entity_load_multiple(array(
    '714700:28476728',
    '714700:29073198',
    '714700:23341743',
    '714700:21820199',
  ));
  $out = array();
  foreach ($mult as $object) {
    $out[] = ding_entity_view($object, 'big_teaser');
  }

  return $out;
}

function ding_list_get_block_your_lists() {
  global $user;
  $uid = ($user->uid != 0) ? $user->uid : NULL;
  $list_page = (bool)(arg(1) == 'manage-lists');
  
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'your_lists',
    '#title' => t('Mine lister', array(), array('langcode' => 'da')),
    '#navigation' => 'What is navigation',
    '#mode_tabs' => array(),
    '#tabs' => array(
      // Inspiration
      // Recent
      'lists' => array(
        '#theme' => 'ding_list_content',
        '#title' => '',
        '#content' => 'Henter dine lister...',
        '#type' => 'view',
        '#view_name' => 'ding_lists',
        '#view_display' => 'block_page', //$list_page ? 'block_page' : 'default',
        '#view_args' => array('uid' => $uid),
        '#no_tab_link' => !$list_page
      ),
    )
  );
  
  if ($list_page) {
    $result['#more_link'] = "user/create-list";
    $result['#more_title'] = "Tilføj ny liste";
  }
  try {
    $creds = ding_user_get_creds();
    if (isset($creds['name'])) {
      $consent = ding_user_consent_get_consent();
      if (isset($consent['loan_history_store']) && $consent['loan_history_store'] == 0) {
        $content = render(messages_show(4));
        
        $result = array(
          '#theme' => 'ding_list',
          '#name' => 'your_lists',
          '#title' => t('Mine lister', array(), array('langcode' => 'da')),
          '#navigation' => 'What is navigation',
          '#more_title' => 'Læs mere her',
          '#more_link' => 'page/samtykke',
          '#mode_tabs' => array(),
          '#tabs' => array(
            'lists' => array(
              '#theme' => 'ding_list_content',
              '#title' => '',
              '#content' => $content,
              '#type' => 'render',
              '#no_tab_link' => TRUE,
            ),
          ),
        );
      }
    }
  } catch(Exception $e) {
    $content = render(messages_show(4));
     
    $result = array(
      '#theme' => 'ding_list',
      '#name' => 'your_lists',
      '#title' => t('Missing consent'),
      '#navigation' => 'What is navigation',
      '#more_title' => 'Læs mere her',
      '#more_link' => 'page/samtykke',
      '#mode_tabs' => array(),
      '#tabs' => array(
        'lists' => array(
          '#theme' => 'ding_list_content',
          '#title' => '',
          '#content' => $content,
          '#type' => 'render',
          '#no_tab_link' => TRUE,
        ),
      ),
    );
  }
  return $result;
}

function ding_list_get_block_ding_list() {
  global $user;
  
  switch (arg(1)) {
    case 'list':
      $entity = entity_load_single('ding_type', arg(2));
    break;
    case 'history':
      $entity = ding_list_get_list(DING_LIST_TYPE_LOAN_HISTORY, $user);
      $entity->title = 'Bøger og andre materialer jeg har lånt';
    break;
  }
  
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'ding_list',
    '#title' => $entity->title,
    '#mode_tabs' => array(
        array(t('Overview list'), 'list_item'),
        array(t('Compact list'), 'list_item_thin'),
      ),
    '#tabs' => array(
      'lists' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'List',
        '#content' => 'Henter liste',
        '#type' => 'user_list',
        '#list_id' => $entity->id,
        '#no_tab_link' => TRUE,
      ),
    )
  );
  try {
    $creds = ding_user_get_creds();
    if (isset($creds['name'])) {
      $consent = ding_user_consent_get_consent();
      if (isset($consent['loan_history_store']) && $consent['loan_history_store'] == 0) {
        $content = render(messages_show(4));
        $result = array(
          '#theme' => 'ding_list',
          '#name' => 'ding_list',
          '#title' => t('Missing consent'),
          '#more_title' => 'Læs mere her',
          '#more_link' => 'page/samtykke',
          '#mode_tabs' => array(),
          '#tabs' => array(
            'lists' => array(
              '#theme' => 'ding_list_content',
              '#title' => 'List',
              '#content' => $content,
              '#no_tab_link' => TRUE,
            ),
          )
        );
      }
    }
  } catch(Exception $e) {
    $content = render(messages_show(4));
    $result = array(
      '#theme' => 'ding_list',
      '#name' => 'ding_list',
      '#title' => t('Missing consent'),
      '#more_title' => 'Læs mere her',
      '#more_link' => 'page/samtykke',
      '#mode_tabs' => array(),
      '#tabs' => array(
        'lists' => array(
          '#theme' => 'ding_list_content',
          '#title' => 'List',
          '#content' => $content,
          '#no_tab_link' => TRUE,
        ),
      )
    );
  }
  return $result;
}

function ding_list_get_block_debt_list() {
  global $user, $language;
  $debts = ding_provider_invoke_page('debt', 'list', $user);
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'debt_list',
    '#title' => t('My debts'),
    '#mode_tabs' => array(),
    '#tabs' => array(
      'debt_list' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'main',
        '#type' => 'render',
        '#content' => array(
          'form' => array(
            ding_provider_get_form('ding_debt_debts_form', $debts)
          ),
          // 'terms' => array(
            // '#theme' => 'dibs_terms_of_sale',
            // '#terms_of_sale' => variable_get('terms_of_sale_' . $language->language),
            // '#prefix' => '<h2>' . t('Terms and conditions of sale') . '</h2>'
          // ),
          // Adding 'dibs-cards-supported' requires getting the supported cards
          // see the ding_dibs.module
        ),
        '#no_tab_link' => TRUE,
      ),
    )
  );
  
  return $result;
}

function ding_list_get_block_user_edit() {
  global $user, $language;
  $account = menu_get_object('user', 1);
  if(!isset($account))
    $account = $user;
  
  $user_form = drupal_get_form('user_profile_form', $account);
  $form_data = array();
  foreach (element_children($user_form) as $element) {
    $form_data[$element] = $user_form[$element];
  }
  
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'user_edit',
    '#title' => t('Edit user profile'),
    '#mode_tabs' => array(),
    '#tabs' => array(
      'user_edit' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'main',
        '#type' => 'render',
        '#content' => array(
          $form_data,
        ),
        '#no_tab_link' => TRUE,
      ),
    )
  );
  
  return $result;
}

function ding_list_get_block_loan_list() {
  global $user;
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'loan_list',
    '#title' => 'Hjemlån',
    '#mode_tabs' => array(
        array(t('Overview list'), 'list_item'),
        array(t('Compact list'), 'list_item_thin'),
      ),
    '#tabs' => array(
      'main_loan' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'main',
        '#type' => 'render',
        '#content' => array(
          'list' => array(
            '#theme' => 'main_loan_list',
            '#account' => user_load($user->uid),
            '#view_mode' => 'list_item'
          )
        ),
        '#no_tab_link' => TRUE,
      ),
    )
  );
  
  return $result;
}

function ding_list_get_block_reserved_list() {
  global $user;
  $result = array(
    '#theme' => 'ding_list',
    '#name' => 'reserved_list',
    '#title' => t('My reservations'),
    '#mode_tabs' => array(
        array(t('Overview list'), 'list_item'),
        array(t('Compact list'), 'list_item_thin'),
      ),
    '#tabs' => array(
      'ready' => array(
        '#theme' => 'ding_list_content',
        '#title' => t('Ready for pickup'),
        '#type' => 'render',
        '#content' => array(
          'list' => array(
            '#theme' => 'main_reservation_list',
            '#account' => user_load($user->uid),
            '#view_mode' => 'list_item',
            '#ready_for_pickup' => TRUE,
          )
        ),
      ),
      'not_ready' => array(
        '#theme' => 'ding_list_content',
        '#title' => t('Not arrived'),
        '#type' => 'render',
        '#content' => array(
          'list' => array(
            '#theme' => 'main_reservation_list',
            '#account' => user_load($user->uid),
            '#view_mode' => 'list_item',
            '#ready_for_pickup' => FALSE,
          )
        ),
      ),
    )
  );
  
  return $result;
}

function ding_list_get_block_your_activities() {
  global $user;
  $uid = ($user->uid != 0) ? $user->uid : NULL;
  return array(
    '#theme' => 'ding_list',
    '#name' => 'your_activities',
    '#title' => 'Emneord jeg følger',
    '#navigation' => 'What is navigation',
    '#mode_tabs' => array(),
    '#tabs' => array(
      // Inspiration
      // Recent
      'lists' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'Mine lister',
        '#no_tab_link' => TRUE,
        '#content' => 'Henter dine aktiviteter...',
        '#type' => 'view',
        '#view_name' => 'ding_lists_terms',
        '#view_display' => 'block',
        '#view_args' => array('uid' => $uid),
      ),
    )
  );
}

function ding_list_get_block_inspiration() {
  $tabs = array(
    '#theme' => 'ding_list',
    '#name' => 'inspiration',
    '#title' => 'Inspiration',
    '#navigation' => 'My navigation',
    '#tabs' => array(
      'object' => array(
        '#title' => 'Inspiration til dig',
        '#theme' => 'ding_list_content',
        '#type' => 'serendipity',
        '#ajax' => TRUE,
        '#options' => array(
          'context' => array(
            'target' => 'inspiration ding_objects',
            'frontpage' => drupal_is_front_page(), 
            'bundle' => 'ting_object',
            'all_user_lists' => TRUE,
            'taxonomy_tid' => '[term:tid]',
            'recommended' => TRUE,
            'view_mode' => 'grid_item',
            'random' => TRUE,
            'max' => 40,
            'min' => 4,
          ),
        ),
        '#limit' => 4,
      ),
      'recent' => array(
        '#title' => 'Nyeste',
        '#theme' => 'ding_list_content',
        '#type' => 'serendipity',
        '#ajax' => TRUE,
        '#options' => array(
          'context' => array(
            'target' => 'recent ding_objects',
            'frontpage' => drupal_is_front_page(), 
            'bundle' => 'ting_object',
            'recent' => TRUE,
            'taxonomy_tid' => '[term:tid]',
            'view_mode' => 'grid_item',
            'random' => TRUE,
            'max' => 80,
            'min' => 4,
          ),
        ),
        '#limit' => 4,
      ),
    )
  );
  if($term = menu_get_object('taxonomy_term', 2)) {
    if(
      isset($term->field_popular_ting_objects[field_language('taxonomy_term', $term, 'field_popular_ting_objects')]) &&
      count($term->field_popular_ting_objects[field_language('taxonomy_term', $term, 'field_popular_ting_objects')])) {
      $tabs['#tabs']['popular'] = array(
        '#theme' => 'ding_list_content',
        '#title' => 'Mest udlånte',
        '#type' => 'serendipity',
        '#ajax' => FALSE,
        '#name' => 'popular',
        '#options' => array(
          'context' => array(
            'popular' => TRUE,
            'taxonomy_tid' => '[term:tid]',
            'view_mode' => 'grid_item',
            'random' => TRUE,
            'max' => 40,
            'min' => 4,
          )
        ),
        '#limit' => 4,
      );
    }
    if($term->tid != 5) {
      $tabs['#title'] = t('Materialer under @title', array('@title' => $term->name), array('langcode' => 'da'));
    }
  }
  return $tabs;
}

/*
* This should be a view when ting_object reference is fixed
*
*
*/
function ding_list_get_popular() {
  global $user;
  if ($user->uid == 1) {

    
    if (arg(0) == "taxonomy" && arg(1) == "term") {
      $tid = arg(2); 
    }
    $tid = drupal_is_front_page() ? 5 : $tid;
    $term = taxonomy_term_load($tid);
    $objects = ding_serendipity_taxonomy_term_fetch_ting_objects('field_popular_ting_objects',$term);

    return ding_list_view_multiple_ding_entities($objects);
  }
}

function ding_list_view_multiple_ding_entities($objects) {
    $ids = array();
    foreach ($objects as $oid => $object) {
      $ids[] = $object['id'];
    }

    $load = ding_entity_load_multiple($ids);

    $out = array();
    foreach ($load as $oid => $object) {
      $out[] = ding_entity_view($object, 'big_teaser');
    }
    
    return $out;
}

function ding_list_get_block_calendar() {
  global $user;
  $term = main_subterms_get_current_term();
  $tid = !empty($term->tid) ? $term->tid : 5;
  $view_all_link = 'events' . ((!empty($term)) ? '/' . $term->name : '');
  return array(
    '#theme' => 'ding_list',
    '#name' => 'calendarinspiration',
    '#title' => 'Kalender',
    '#navigation' => 'My navigation',
    '#more_link' => 'events/kultunaut',
    '#more_title' => 'Se alle events',
    '#tabs' => array(
      // Recent
      /*'inspiration1' => array(
        '#title' => 'Arrangementer for dig',
        '#theme' => 'ding_list_content',
        '#type' => 'render',
        '#content' => ding_serendipity_render(
            array(
              'target' => 'Related events',
              'frontpage' => drupal_is_front_page(), 
              'entity_type' => 'node',
              'bundle' => 'ding_event',
              'taxonomy_tid' => '[term:tid]',
              'related' => TRUE,
            ),
            array(
              'view_mode' => 'teaser',
              'random' => TRUE,
              'max' => 6,
              'min' => 6,
            )
          ),
      ),*/
      'recent2' => array(
        '#title' => 'Nyeste',
        '#theme' => 'ding_list_content',
        '#type' => 'render',
        '#content' => kultunaut_feed_page_content('term'),
      ),
      /*'recent2' => array(
        '#title' => 'Nyeste',
        '#theme' => 'ding_list_content',
        '#content' => 'Henter nyeste bøger...',
        '#type' => 'view',
        '#view_name' => 'events',
        '#view_display' => 'block_2',
        '#view_args' => array('tid' => $tid),
      ),
      'popular2' => array(
        '#title' => 'Mest populære',
        '#theme' => 'ding_list_content',
        '#content' => 'Henter nyeste bøger...',
        '#type' => 'view',
        '#view_name' => 'events',
        '#view_display' => 'block_2',
      ),
      */    
    )
  );
}

function ding_list_get_block_blog() {
  global $user;
  $term = main_subterms_get_current_term();
  $view_all_link = 'blogs/tag' . ((!empty($term)) ? '/' . $term->tid : '');
  $tid = !empty($term->tid) ? $term->tid : 5;
  
  if (!empty($term)) {
    $title = t('Blogs about @tag', array('@tag' => $term->name));
  } else {
    $title = t('Blogs');
  }
  
  $blogs = array(
    '#theme' => 'ding_list',
    '#name' => 'blog',
    '#title' => $title,
    '#navigation' => 'My navigation',
    '#more_link' => $view_all_link,
    '#more_title' => 'Se alle blogindlæg',
    '#tabs' => array(
      // Inspiration
      'inspiration' => array(
        '#title' => 'Blogindlæg for dig',
        '#theme' => 'ding_list_content',
        '#type' => 'serendipity',
        '#options' => array(
            'context' => array(
              'target' => 'Related blog posts',
              'frontpage' => drupal_is_front_page(), 
              'entity_type' => 'node',
              'bundle' => 'blog_post',
              'taxonomy_tid' => '[term:tid]',
              'related' => TRUE,
              'view_mode' => 'big_teaser',
              'random' => TRUE,
              'max' => 2,
            )
          ),
      ),
      'recent_blogs' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'Nyeste blogindlæg',
        '#content' => 'Henter blogindlæg...',
        '#type' => 'view',
        '#view_name' => 'blogs',
        '#view_display' => array(
          'grid_item' => 'block_2',
          'list_item' => 'block_3',
          'list_item_thin' => 'block_4',
        ),
        '#view_args' => array('tid' => $tid),
      ),
    )
  );
  
  
  if (!drupal_is_front_page()) {
    unset($blogs['#tabs']['recent']);
    unset($blogs['#tabs']['inspiration']);
  }
  
  return $blogs;
}

function ding_list_get_block_news() {
  global $user;
  $term = main_subterms_get_current_term();
  $view_all_link = 'news' . ((!empty($term)) ? '/all/' . $term->name : '');
  $tid = !empty($term->tid) ? $term->tid : 5;
  return array(
    '#theme' => 'ding_list',
    '#name' => 'news',
    '#title' => 'Nyheder',
    '#navigation' => 'My navigation',
    '#more_link' => "news",
    '#more_title' => "Se alle nyheder",
    '#tabs' => array(
      // Inspiration
      'object' => array(
        '#title' => 'Nyheder til dig',
        '#theme' => 'ding_list_content',
        '#type' => 'render',
        '#content' => ding_serendipity_render(
            array(
              'target' => 'Related news',
              'frontpage' => drupal_is_front_page(), 
              'entity_type' => 'node',
              'bundle' => 'ding_news',
              'taxonomy_tid' => '[term:tid]',
              'related' => TRUE,
              'view_mode' => 'big_teaser',
              'random' => TRUE,
              'max' => 2,
              'min' => 1,
            )
          ),
      ),
      'recent' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'Nyeste nyheder',
        '#content' => 'Henter nyheder...',
        '#type' => 'view',
        '#view_name' => 'news',
        '#view_display' => array(
          'grid_item' => 'block_1',
          'list_item' => 'block_3',
          'list_item_thin' => 'block_4',
        ),
        '#view_args' => array('tid' => $tid),
      ),
      /*
      'popular' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'Mest kommenterede',
        '#content' => 'Henter nyheder...',
        '#type' => 'view',
        '#view_name' => 'news',
        '#view_display' => 'block_2',
      ),
      */
    )
  );
}  

function ding_list_get_block_related_pages_list() {
  $term = main_subterms_get_current_term();
  $tid = !empty($term->tid) ? $term->tid : 5;
  
  return array(
    '#theme' => 'ding_list',
    '#name' => 'related_pages',
    '#title' => t('Articles about @term', array('@term' => $term->name)),
    '#mode_tabs' => array(),
    '#tabs' => array(
      'default' => array(
        '#theme' => 'ding_list_content',
        '#title' => 'test',
        '#content' => 'tester',
        '#type' => 'view',
        '#link' => FALSE,
        '#view_name' => 'term',
        '#view_display' => array(
          'grid_item' => 'block_1',
        ),
        '#view_args' => array('tid' => $tid),
      )  
    )
  );
}