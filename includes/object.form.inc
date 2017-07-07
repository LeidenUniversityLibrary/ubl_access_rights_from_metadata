i<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);


/**
 * @file
 * Contains form functions for displaying the access rights for one object.
 */


/**
 * Implements hook_form().
 */
function access_rights_view_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/ui');

  $form['ubl_access_rights_from_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Access rights from metadata'),
  );

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  if ($configlocation) {
    $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
    $accessvalue = ubl_access_rights_from_metadata_get_access_rights_value($object, $config);
    if (!$accessvalue) {
      $accessvalue = '<em>' . t('no access value found in metadata of this object') . '</em>';
    }
    $table = '<TR><TH>' . t('Access value') . '</TH><TD>' . $accessvalue . '</TD></TR>';

    $fromobjid = '';
    $accessible = ubl_access_rights_from_metadata_accessible_dsids($object, $fromobjid);
    $accessible = (count($accessible) > 0)?implode(', ', $accessible):'<em>' . t('Undefined') . '</em>';
    if ($fromobjid) {
      $accessible .= ' <em>(from access rights of ' . $fromobjid . ')</em>';
    }
    $table .= '<TR><TH>' . t('Accessible datastreams') . '</TH><TD>' . $accessible . '</TD></TR>';
     
    $fromobjid = '';
    $restricted = ubl_access_rights_from_metadata_restricted_dsids($object, $fromobjid);
    $restricted = (count($restricted) > 0)?implode(', ', $restricted):'<em>' . t('Undefined') . '</em>';
    if ($fromobjid) {
      $restricted .= ' <em>(from access rights of ' . $fromobjid . ')</em>';
    }
    $table .= '<TR><TH>' . t('Restricted datastreams') . '</TH><TD>' . $restricted . '</TD></TR>';
     
    $fromobjid = '';
    $downloadable = ubl_access_rights_from_metadata_downloadable_dsids_with_label($object, $fromobjid);
    $table .= '<TR><TH>' . t('Downloadable datastreams') . '</TH>';
    if (count($downloadable) > 0) {
      $table .= '<TD>';
      foreach ($downloadable as $dsid => $text) {
        $table .= $dsid . ' => ' . $text . '<BR/>'; 
      }
      if ($fromobjid) {
        $table .= ' <em>(from access rights of ' . $fromobjid . ')</em>';
      }
      $table .= '</TD>';
    }
    else {
      $table .= '<TD><em>' . t('None') . '</em>';
      if ($fromobjid) {
        $table .= ' <em>(from access rights of ' . $fromobjid . ')</em>';
      }
      $table .= '</TD>';
    }
    $table .= '</TR>';

    $fromobjid = '';
    $roles = ubl_access_rights_from_metadata_allowed_roles($object, $fromobjid);
    $roles = (count($roles) > 0)?implode(', ', $roles):'<em>' . t('Undefined') . '</em>';
    if ($fromobjid) {
      $roles .= ' <em>(from access rights of ' . $fromobjid . ')</em>';
    }
    $table .= '<TR><TH>' . t('Allowed roles') . '</TH><TD>' . $roles . '</TD></TR>';

    $fromobjid = '';
    $accessdesc = ubl_access_rights_from_metadata_access_description($object, $fromobjid);
    $from = ($fromobjid)?' <em>(from access rights of ' . $fromobjid . ')</em>':'';
    $table .= '<TR><TH>' . t('Access text') . '</TH><TD>' . $accessdesc['accesstext'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access use text') . '</TH><TD>' . $accessdesc['accessusetext'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access link') . '</TH><TD>' . $accessdesc['accesslink'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access image') . '</TH><TD>' . $accessdesc['accessimage'] . $from . '</TD></TR>';

    if (ubl_access_rights_from_metadata_is_ip_dependent($object, $fromobjid)) {
      $ipranges = $config['_config']['internal']['ip_ranges'];
      $isinternal = _ubl_access_rights_from_metadata_client_ip_in_range($ipranges);
      $text = $isinternal?t("Above is for someone who is within the IP range (internal)"):t("Above is for someone external"); 
      $text .= ' (current ip adress is: ' . ip_address() . ')';
      $from = ($fromobjid)?' <em>(from access rights of ' . $fromobjid . ')</em>':'';
      $table .= '<TR><TH>' . t('IP dependent content') . '</TH><TD>' . $text . $from . '</TD></TR>';
    }
    else {
      $from = ($fromobjid)?' <em>(from access rights of ' . $fromobjid . ')</em>':'';
      $table .= '<TR><TH>' . t('IP dependent content') . '</TH><TD>' . t('This is not IP dependent content.') . $from . '</TD></TR>';
    }
  }
  else {
    $table = '<TR><TD>' . t('No configuration location') . '</TD></TR>';
  }
  
  $form['ubl_access_rights_from_metadata']['info'] = array(
    '#markup' => '<TABLE>' . $table . '</TABLE>',
  );

  return $form;
}
