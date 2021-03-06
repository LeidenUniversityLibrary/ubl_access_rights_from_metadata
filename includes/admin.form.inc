<?php

/**
 * @file
 * The admin form for the UBL access rights from metadata module.
 *
 *
 *  Copyright 2017 Leiden University Library
 *
 *  This file is part of ubl_access_rights_from_metadata.
 *
 *  ubl_access_rights_from_metadata is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
  $tn_url = variable_get('ubl_access_rights_from_metadata_tn_url');
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
  variable_set('ubl_access_rights_from_metadata_tn_url', $form_state['values']['tn_url']);
  if ($oldconfiglocation) {
    ubl_access_rights_from_metadata_refresh_configuration($oldconfiglocation);
  }
  drupal_set_message(t('The settings have been saved!'));
}
