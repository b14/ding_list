<?php
/**
 * @file
 * ding_list_installer.features.inc
 */

/**
 * Implements hook_eck_bundle_info().
 */
function ding_list_installer_eck_bundle_info() {
  $items = array(
  'ding_type_ding_list' => array(
  'machine_name' => 'ding_type_ding_list',
  'entity_type' => 'ding_type',
  'name' => 'ding_list',
  'label' => 'ding_list',
),
  'ding_type_ding_list_element' => array(
  'machine_name' => 'ding_type_ding_list_element',
  'entity_type' => 'ding_type',
  'name' => 'ding_list_element',
  'label' => 'Ding List Element',
),
  );
  return $items;
}

/**
 * Implements hook_eck_entity_type_info().
 */
function ding_list_installer_eck_entity_type_info() {
$items = array(
       'ding_type' => array(
  'name' => 'ding_type',
  'label' => 'ding_type',
  'properties' => array(
    'title' => array(
      'label' => 'Title',
      'type' => 'text',
      'behavior' => 'title',
    ),
    'uid' => array(
      'label' => 'Author',
      'type' => 'integer',
      'behavior' => 'author',
    ),
    'created' => array(
      'label' => 'Created',
      'type' => 'integer',
      'behavior' => 'created',
    ),
    'changed' => array(
      'label' => 'Changed',
      'type' => 'integer',
      'behavior' => 'changed',
    ),
    'more' => array(
      'label' => 'More',
      'type' => 'text',
      'behavior' => '',
    ),
    'openlist_id' => array(
      'label' => 'Openlist ID',
      'type' => 'integer',
      'behavior' => NULL,
    ),
  ),
),
  );
  return $items;
}
