<?php

/**
 * @file
 * Contains form functions for displaying the access rights for one object.
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
 * Implements hook_form().
 */
function access_rights_view_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_settings');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/ui');

  $form['ubl_access_rights_from_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Access rights from metadata'),
  );

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  if ($configlocation) {
    $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
    $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($object, $config, NULL); 
    $from = '';
    if (isset($accesssettings['source_of_settings']) && strlen($accesssettings['source_of_settings']) > 0) {
      $from = ' <em>(from access rights of ' . $accesssettings['source_of_settings'] . ')</em>';
    }
    $table = '<TR><TH>' . t('Conditions met') . '</TH><TD>' . (empty($accesssettings['_conditions_met'])?"<em>(none)</em>":implode(",", $accesssettings['_conditions_met'])) . $from . '</TD></TR>';

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
    $accessdesc = ubl_access_rights_from_metadata_access_description($object, $fromobjid);
    $from = ($fromobjid)?' <em>(from access rights of ' . $fromobjid . ')</em>':'';
    $table .= '<TR><TH>' . t('Access text') . '</TH><TD>' . $accessdesc['accesstext'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access use text') . '</TH><TD>' . $accessdesc['accessusetext'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access link') . '</TH><TD>' . $accessdesc['accesslink'] . $from . '</TD></TR>';
    $table .= '<TR><TH>' . t('Access image') . '</TH><TD>' . $accessdesc['accessimage'] . $from . '</TD></TR>';
  }
  else {
    $table = '<TR><TD>' . t('No configuration location') . '</TD></TR>';
  }
  
  $form['ubl_access_rights_from_metadata']['info'] = array(
    '#markup' => '<TABLE>' . $table . '</TABLE>',
  );

  return $form;
}
