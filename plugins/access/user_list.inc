<?php

/**
 * @file
 * Plugin to provide access control based on user lists.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("User: list"),
  'description' => t('Control access by to user lists.'),
  'callback' => 'ding_list_user_list_ctools_access_check',
  'default' => 1,
  'settings form' => 'ding_list_user_list_ctools_access_settings',
  'summary' => 'ding_list_user_list_ctools_access_summary',
  'required context' => new ctools_context_required(t('User'), 'user'),
);

/**
 * Settings form for the 'by perm' access plugin
 */
function ding_list_user_list_ctools_access_settings($form, &$form_state, $conf) {
  $form['settings']['user_is_owner'] = array(
    '#type' => 'checkbox',
    '#title' => t('The list is owned by the current user'),
    '#default_value' => 1,
    '#description' => t('Control if list owners or only other users may view a list.'),
  );

  return $form;
}

/**
 * Check for access.
 */
function ding_list_user_list_ctools_access_check($conf, $context) {
  if (empty($context) || empty($context->data)) {
    return FALSE;
  }
  $result = ding_list_get_list(arg(2), $context->data);
  if ($conf['user_is_owner']) {
    return $result->uid == $context->data->uid;
  }
  else {
    return $result->uid != $context->data->uid;
  }
}

/**
 * Provide a summary description based upon the checked roles.
 */
function ding_list_user_list_ctools_access_summary($conf, $context) {
  if (!isset($conf['user_is_owner'])) {
    return t('Error, ´choose your setting.');
  }
  
  return t('@identifier @is_owner owner', array('@identifier' => $context->identifier, '@is_owner' => ($conf['user_is_owner']) ? 'is' : 'is not'));
}

