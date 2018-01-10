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

    $table = '<TR><TH>' . t('Conditions met') . '</TH><TD>';
    if(empty($accesssettings['_conditions_met'])) {
      $table .= "<em>(none)</em>";
    }
    else {
      foreach ($accesssettings['_conditions_met'] as $cond => $isaccesscond) {
        if ($isaccesscond) {
          $table .= '<strong>' . $cond . '</strong>';
        }
        else {
          $table .= $cond;
        }
        $table .= ' ';
      }
    } 
    $table .= '</TD></TR>';

    $accessible = ubl_access_rights_from_metadata_accessible_object($object);
    $accessible = (($accessible !== NULL)?($accessible?"FALSE":"TRUE"):'<em>' . t('Undefined') . '</em>');
    $accessible .= ' ' . _access_rights_view_reason($accesssettings, 'deny_viewing', $object->id);
    $table .= '<TR><TH>' . t('Deny viewing') . '</TH><TD>' . $accessible . '</TD></TR>';

    $accessible = ubl_access_rights_from_metadata_accessible_dsids($object);
    $accessible = (count($accessible) > 0)?implode(', ', $accessible):'<em>' . t('None') . '</em>';
    $accessible .= ' ' . _access_rights_view_reason($accesssettings, 'allow_access_to_dsid', $object->id);
    $table .= '<TR><TH>' . t('Accessible datastreams') . '</TH><TD>' . $accessible . '</TD></TR>';
     
    $fromobjid = '';
    $restricted = ubl_access_rights_from_metadata_restricted_dsids($object);
    $restricted = (count($restricted) > 0)?implode(', ', $restricted):'<em>' . t('None') . '</em>';
    $restricted .= ' ' . _access_rights_view_reason($accesssettings, 'deny_access_to_dsid', $object->id);
    $table .= '<TR><TH>' . t('Restricted datastreams') . '</TH><TD>' . $restricted . '</TD></TR>';
     
    $fromobjid = '';
    $downloadable = ubl_access_rights_from_metadata_downloadable_dsids_with_label($object);
    $table .= '<TR><TH>' . t('Downloadable datastreams') . '</TH>';
    if (count($downloadable) > 0) {
      $table .= '<TD>';
      foreach ($downloadable as $dsid => $text) {
        $table .= $dsid . ' => ' . $text . '<BR/>'; 
      }
      $table .= ' ' . _access_rights_view_reason($accesssettings, 'provide_download_of_dsid', $object->id);
      $table .= '</TD>';
    }
    else {
      $table .= '<TD><em>' . t('None') . '</em>';
      $table .= ' ' . _access_rights_view_reason($accesssettings, 'provide_download_of_dsid', $object->id);
      $table .= '</TD>';
    }
    $table .= '</TR>';

    $fromobjid = '';
    $accessdesc = ubl_access_rights_from_metadata_access_description($object);
    $from = _access_rights_view_reason($accesssettings, 'access_text', $object->id);
    $table .= '<TR><TH>' . t('Access text') . '</TH><TD>' . $accessdesc['accesstext'] . $from . '</TD></TR>';
    $from = _access_rights_view_reason($accesssettings, 'access_usetext', $object->id);
    $table .= '<TR><TH>' . t('Access use text') . '</TH><TD>' . $accessdesc['accessusetext'] . $from . '</TD></TR>';
    $from = _access_rights_view_reason($accesssettings, 'access_link', $object->id);
    $table .= '<TR><TH>' . t('Access link') . '</TH><TD>' . $accessdesc['accesslink'] . $from . '</TD></TR>';
    $from = _access_rights_view_reason($accesssettings, 'access_image', $object->id);
    $table .= '<TR><TH>' . t('Access image') . '</TH><TD>' . $accessdesc['accessimage'] . $from . '</TD></TR>';
    $ipdependent = (isset($accesssettings['_ip_dependent']) && $accesssettings['_ip_dependent'])?t('yes'):t('no');
    $table .= '<TR><TH>' . t('IP dependent') . '</TH><TD>' . $ipdependent . ' (current ip address is: ' . ip_address() . ')</TD></TR>';
  }
  else {
    $table = '<TR><TD>' . t('No configuration location') . '</TD></TR>';
  }
  
  $form['ubl_access_rights_from_metadata']['info'] = array(
    '#markup' => '<TABLE>' . $table . '</TABLE>',
  );

  return $form;
}


function _access_rights_view_reason($accesssettings, $type, $forobjectid) {
  $reason = '';
  if (isset($accesssettings['_reasons'][$type])) {
    foreach ($accesssettings['_reasons'][$type] as $condition => $objectid) {
      if (strlen($reason) > 0) {
        $reason .= ', ';
      }
      if ($objectid === 'default') {
        $reason .= t('of default value');
      }
      else {
        if ($objectid === $forobjectid) {
          $reason .= t('condition "%condition" is met', array('%condition' => $condition));
        }
        else {
          $reason .= t('condition "%condition" is met for object "%object"', array('%condition' => $condition, '%object' => $objectid));
        }
      } 
    }
    if (strlen($reason) > 0) {
      $reason = ' (<em>' . t('because') . ' ' . $reason . '</em>)';
    }
  }
  return $reason;
}
