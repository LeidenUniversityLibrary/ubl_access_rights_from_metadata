<?php

/**
 * @file
 * Retrieve the access rights value from an object, using caching.
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

function ubl_access_rights_from_metadata_get_access_rights_value($obj, $config, $rebuildcache = FALSE) {
  $id2acvalue = &drupal_static(__FUNCTION__, array());
  $id = $obj->id;
  $lastchangedate = (string)$obj->lastModifiedDate;
  if ($rebuildcache) {
    unset($id2acvalue[$id]);
  }
  if (!array_key_exists($id, $id2acvalue)) {
    $cachename = $id;
    $cachebin = 'cache_ubl_access_rights_from_metadata_access_rights_value';
    if (!$rebuildcache) {
      if ($cache = cache_get($cachename, $cachebin)) {
        $found = $cache->data;
        if ($found['date'] === $lastchangedate) {
          $id2acvalue[$id] = $cache->data;
        }
      }
    }
    if (!array_key_exists($id, $id2acvalue)) {
      module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
      $rightssourcesettings = ubl_access_rights_from_metadata_get_rights_source_settings($config);
      $dsid = $rightssourcesettings['dsid'];
      $id2acvalue[$id] = array('arv' => NULL, 'date' => $lastchangedate);
      if (isset($obj[$dsid])) {
        $xpaths = $rightssourcesettings['xpath'];
        if (!is_array($xpaths)) {
          $xpaths = array(0 => $xpaths);
        }
        $remove = array();
        if (isset($rightssourcesettings['remove'])) {
          $remove = $rightssourcesettings['remove'];
          if (!is_array($remove)) {
            $remove = array(0 => $remove);
          }
        }
        
        $content = $obj[$dsid]->content;
        $domdoc = new DOMDocument();
        if ($domdoc->loadXML($content)) {
          $id2acvalue[$id]['arv'] = ''; // subtle change to indicate that the DS XML exists, but does not have any access rights info
          $domxpath = new DOMXPath($domdoc);
          foreach ($rightssourcesettings['namespace'] as $ns) {
            list($prefix, $uri) = explode(' ', $ns, 2);
            $domxpath->registerNamespace($prefix, $uri);
          }
          foreach ($xpaths as $index => $xpath) {
            $domnodelist = $domxpath->query($xpath);
            if ($domnodelist && $domnodelist->length > 0) {
              foreach ($domnodelist as $domnode) {
                $text = $domnode->textContent;
                if (isset($remove[$index]) && strlen($remove[$index]) > 0) {
                  $text = str_replace($remove[$index], '', $text);
                }
                if (isset($text) && strlen($text) > 0) {
                  if (strlen($id2acvalue[$id]['arv']) > 0) {
                    $id2acvalue[$id]['arv'] .= '--';
                  }
                  $id2acvalue[$id]['arv'] .= $text;
                }
              }
            }
          }
        }
      }
      cache_set($cachename, $id2acvalue[$id], $cachebin);
    }
  }
  return $id2acvalue[$id]['arv'];
}
