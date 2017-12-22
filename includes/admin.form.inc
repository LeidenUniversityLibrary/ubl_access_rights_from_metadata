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
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/ui');

  $form = array();

  if (isset($form_state['input']['config_location']) && strlen($form_state['input']['config_location']) > 0) {
    $config_location = $form_state['input']['config_location'];
  }
  else {
    $config_location = variable_get('ubl_access_rights_from_metadata_config_location');
  }
  $tn_url = variable_get('ubl_access_rights_from_metadata_tn_url');
  $enable_xacml = variable_get('ubl_access_rights_from_metadata_enable_xacml', FALSE);
  $defaults = ubl_access_rights_from_metadata_default_settings();

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
  $form['config']['tn_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Access restricted thumbnail URL'),
    '#description' => t('A URL (absolute or relative to the server) of a thumbnail to use when the current item is not accessible by the user.'),
    '#default_value' => $tn_url,
  );
  $form['config']['enable_xacml'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable XACML'),
    '#description' => t('Also changes the XACML based on the metadata. Enable this if users can get to content via Fedora.'),
    '#default_value' => $enable_xacml,
  );
  $form['config']['defaults'] = array(
    '#type' => 'fieldset',
    '#title' => t('Defaults'),
  );
  foreach ($defaults as $defaultkey => $defaultsetting) {
    $default = variable_get('ubl_access_rights_from_metadata_default_' . $defaultkey, '');
    if ($defaultsetting['type'] === 'dsids') {
      $default = implode(',', $default);
    }
    $form['config']['defaults'][$defaultkey] = array(
      '#type' => 'textfield',
      '#title' => $defaultsetting['title'],
      '#description' => isset($defaultsetting['description'])?$defaultsetting['description']:'',
      '#default_value' => $default,
    );
  }
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
  $defaults = ubl_access_rights_from_metadata_default_settings();
  foreach ($defaults as $defaultkey => $defaultsetting) {
    $value = $form_state['values'][$defaultkey];
    if (empty($value)) {
      // empty defaults are not an error.
    }
    elseif ($defaultsetting['type'] === 'dsids') {
      $dsids = explode(',', $value);
      foreach ($dsids as $dsid) {
        if (preg_match('/^[A-Z][A-Z0-9]+$/', $dsid) !== 1) {
          form_set_error($defaultkey, t("Datastream ID '%dsid' is not correct", array("%dsid" => $dsid)));
        }
      }
    }
    elseif($defaultsetting['type'] === 'url') {
      if (filter_var($value, FILTER_VALIDATE_URL) === FALSE) {
        form_set_error($defaultkey, t("%title should be a URL", array("%title" => $defaultsetting['title'])));
      }
    }
  }
}

function ubl_access_rights_from_metadata_admin_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/ui');

  $oldconfiglocation = variable_get('ubl_access_rights_from_metadata_config_location');
  variable_set('ubl_access_rights_from_metadata_config_location', $form_state['values']['location']);
  variable_set('ubl_access_rights_from_metadata_enable_xacml', $form_state['values']['enable_xacml']);
  variable_set('ubl_access_rights_from_metadata_tn_url', $form_state['values']['tn_url']);
  if ($oldconfiglocation) {
    ubl_access_rights_from_metadata_refresh_configuration($oldconfiglocation);
  }
  $defaults = ubl_access_rights_from_metadata_default_settings();
  foreach ($defaults as $defaultkey => $defaultsetting) {
    if ($defaultsetting['type'] === 'dsids') {
      variable_set('ubl_access_rights_from_metadata_default_' . $defaultkey, explode(',', $form_state['values'][$defaultkey]));
    }
    else {
      variable_set('ubl_access_rights_from_metadata_default_' . $defaultkey, $form_state['values'][$defaultkey]);
    }
  }

  drupal_set_message(t('The settings have been saved!'));
}
