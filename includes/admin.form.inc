<?php

/**
 * @file
 * The admin form for the UBL access rights from metadata module.
 */

/**
 * Implementation of a form. 
 *
 * @see drupal_get_form
 */

function ubl_access_rights_from_metadata_admin_form(array $form, array &$form_state) {
  $form = array();

  if (isset($form_state['input']['config_location']) && strlen($form_state['input']['config_location']) > 0) {
    $config_location = $form_state['input']['config_location'];
  }
  else {
    $config_location = variable_get('ubl_access_rights_from_metadata_config_location');
  }
  $enable_xacml = variable_get('ubl_access_rights_from_metadata_enable_xacml', FALSE);

  $form['config'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configuration'),
  );
  $form['config']['location'] = array(
    '#type' => 'textfield',
    '#title' => t('Configuration location'),
    '#description' => t('An absolute file path or URL to the configuration file.'),
    '#default_value' => $config_location,
  );
  $form['config']['enable_xacml'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable XACML'),
    '#description' => t('Also changes the XACML based on the metadata. Enable this if users can get to content via Fedora.'),
    '#default_value' => $enable_xacml,
  );
  $form['form_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

function ubl_access_rights_from_metadata_admin_form_validate(array $form, array &$form_state) {
  $newloc = $form_state['values']['location'];

  if (strlen($newloc) > 0) {
    module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
    $error = '';
    if (ubl_access_rights_from_metadata_valid_config_file($newloc, $error) === FALSE) {
      form_set_error('location', $error);
    }
  }
}

function ubl_access_rights_from_metadata_admin_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  $oldconfiglocation = variable_get('ubl_access_rights_from_metadata_config_location');
  variable_set('ubl_access_rights_from_metadata_config_location', $form_state['values']['location']);
  variable_set('ubl_access_rights_from_metadata_enable_xacml', $form_state['values']['enable_xacml']);
  if ($oldconfiglocation) {
    ubl_access_rights_from_metadata_refresh_configuration($oldconfiglocation);
  }
  drupal_set_message(t('The settings have been saved!'));
}
