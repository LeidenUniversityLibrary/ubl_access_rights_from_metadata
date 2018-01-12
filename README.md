Deprecated, see for version 2: https://github.com/LeidenUniversityLibrary/islandora_conditional_access_rights

# UBL access rights from metadata

## Introduction

UBL Access rights from metadata lets you define the access rights of an object from a value of the metadata of that object or of one of its parents.
Optionally the XACML policy is changed based on the metadata of the object.
It also defines which datastreams are downloadable.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)

## Installation
 
Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

At `admin/islandora/tools/ubl_access_rights_from_metadata` the configuration location can be defined. This should be an absolute file path
or a URL to the configuration file. See 'ini file' for more information.

The 'Enable XACML' checkbox enables the modification of the XACML policy of the object whose metadata has changed. This is only useful if the
objects can be accessed directly inside Fedora and you want to have access rights in Fedora as well. However, this will have some drawbacks,
because everytime the metadata changes, potentially the XACML policy of the object (and all of its children!) is changed as well.
If you only access your objects through Drupal, there is no need to enable XACML restrictions.
PLEASE NOTE: IP range restrictions can only be inforced through Islandora and not through XACML policies, so these will never have XACML policies.

### ini file

The configuration file is a file in the PHP INI format, see `https://en.wikipedia.org/wiki/INI_file`
for more information about this format. PHP has some special features, such as support for multiple values for one key by using key[] or key[attr].
See the `example_ini_files` directory for examples.

The configuration file should have the following sections:

The section [general] is for general configuration, such as the IP range (`ip_range`) of the server(s) Islandora runs on, so that
these servers always have access to the datastreams if needed. Also, a translation from DSID to a label is defined here (dsid2label).

The section [rightssource] configures the source of the metadata value to use as the base of the rights.
The value is constructed in the following manner: the datastream identified by the key "dsid" is used as XML. The (repeating)
key "xpath" gives one or more XPaths to the data field within that datastream. These values are concatenated with '--' as a
separator. The key "namespace" (repeating) is used for the namespace declaration and consists of a namespace prefix, space,
and the a namespare URI. This is how the rights value is constructed.
Optionally, a (repeating) key "remove" can be defined that contains the string to be removed from the value indicated by the "xpath" key.

There should be a section named after this value to handle the rights. This is called a rights value section.
The section name always begins with "all::" or  "external::" or "internal::". The first ("all::") handles the rights in every
case. The "external::" and "internal::" is used to allow for IP range restrictions, where "internal::" settings are used when
the user is within the IP ranges specified in the [internal] section.

The section [otherwise] handles the case of other metadata values or missing metadata values.
The rights value sections can contain the following keys:
 - `deny_access_to_dsid[]` : repeatable, deny access to the DSID mentioned (this is also done via XACML for non-IP-dependent access rights if this is enabled).
 - `allow_access_to_dsid[]` : repeatable, allow access to the DSID mentioned
 - `allow_access_for_role[]` : repeatable, the user role that has access to all datastreams.
 - `provide_download_of_dsid[]` : repeatable, provide a download button for this datastream. Should not be in `deny_access_to_dsid`. Should be in `allow_access_to_dsid` to make sure the download is possible.
The values of the "`*_to_dsid`" and "`provide_download_of_dsid`" keys should be valid DSID's, or ALL (for all datastreams) or NONE (for no datastream at all).

Below examples of the parts of the configuration file:

The general section where the IP range of the Islandora server is defined and some of the datastream labels:

```
[general]
; ip range for the Islandora server(s)
ip_range[] = "192.0.2.0-192.0.2.1"
dsid2label[MODS] = "Metadata (MODS XML)"
dsid2label[DC] = "Metadata (Dublin Core XML)"
dsid2label[OBJ] = "Original ({mime})"
```


The rightssource section defines where the access value can be found:

```
; rightssource section
[rightssource]
dsid = "MODS"
xpath[] = "/mods:mods/mods:accessCondition[@type='restriction on access']"
xpath[] = "/mods:mods/mods:accessCondition[@type='use and reproduction']/@xlink:href"
namespace[] = "mods http://www.loc.gov/mods/v3"
namespace[] = "xlink http://www.w3.org/1999/xlink"
remove[] = "."
```

Access rights with a download provided of a JPG datastream:

```
[all::Download provided--https://creativecommons.org/licenses/by/4.0/]
deny_access_to_dsid[] = "OBJ"
allow_access_to_dsid[] = "JPG"
provide_download_of_dsid[] = "JPG"
allow_access_for_role[] = "administrator"
access_text = "Download provided."
access_usetext = "Use of this resource is governed by the terms and conditions of the Creative Commons CC-BY License"
access_link = "https://creativecommons.org/licenses/by/4.0/"
access_image = "https://licensebuttons.net/l/by/3.0/88x31.png"
```

Access rights with difference between internal and external:

```
[internal::Access within the library premises--http://rightsstatements.org/vocab/CNE/1.0/]
deny_access_to_dsid[] = "OBJ"
allow_access_to_dsid[] = "JPG"
provide_download_of_dsid[] = "JPG"
allow_access_for_role[] = "administrator"
access_text = "Access within the library premises."
access_usetext = "Copyright not evaluated"
access_link = "http://rightsstatements.org/vocab/CNE/1.0/"
access_image = "http://rightsstatements.org/files/buttons/CNE.dark-white-interior.png"

[external::Access within the library premises--http://rightsstatements.org/vocab/CNE/1.0/]
deny_access_to_dsid[] = "ALL"
allow_access_to_dsid[] = "NONE"
provide_download_of_dsid[] = "NONE"
allow_access_for_role[] = "administrator"
access_text = "Access within the library premises."
access_usetext = "Copyright not evaluated"
access_link = "http://rightsstatements.org/vocab/CNE/1.0/"
access_image = "http://rightsstatements.org/files/buttons/CNE.dark-white-interior.png"
```

Define what is internal to us:

```
; internal section, for defining IP ranges
[internal]
ip_range[] = "198.51.100.0-198.51.100.24"
ip_range[] = "203.0.113.0-203.0.113.24"
```

Otherwise settings (no downloads possible, and copyright is not evaluated):

```
; the rights value section in all other cases
[all::otherwise]
provide_download_of_dsid[] = "NONE"
access_text = "Access restricted."
access_usetext = "Copyright not evaluated"
access_link = "http://rightsstatements.org/vocab/CNE/1.0/"
access_image = "http://rightsstatements.org/files/buttons/CNE.dark-white-interior.png"
```

## Documentation

Because this module also provides a download button, the code below should be included in the template file.

### How to include download buttons

```php
// Render the detail tools block
  $block = module_invoke_all('detail_tools_block_view');

  $block['list']['#type'] = 'ul';
  $block['list']['#theme'] = 'item_list';

  if (isset($block['list']['#attributes']['class'])) {
    $block['list']['#attributes']['class'] = array_unique($block['list']['#attributes']['class']);
  }

  print render($block);

```

## drush commands

### change\_xacml\_policy\_based\_on\_metadata

Change the XACML policy based on the metadata of the objects. An absolute path to a configuration directory with ini files or a single ini file should be supplied. Also a collection ID is mandatory.
This will give a warning when 'Enable XML' is off.

```
drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/path/to/a/ini/file
drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/url/to/a/ini/file
drush --user=admin cxpbomd --collection=islandora:root --configuration=/url/to/a/ini/file
```

### check\_access\_rights\_value

Check the access rights value of the children of a specific collection.

```
drush --user=admin check_access_rights_value --collection=islandora:root --configuration=/path/to/a/ini/file
drush --user=admin check_access_rights_value --collection=islandora:root --configuration=/url/to/a/ini/file
drush --user=admin carv --collection=islandora:root --configuration=/url/to/a/ini/file
```


## Maintainers/Sponsors

Current maintainers:

* [Lucas van Schaik](https://github.com/lucasvanschaik)

## Development

If you would like to contribute to this module, please contact the current maintainer.


## License

[GPLv3](LICENSE.txt)
Copyright 2017 Leiden University Library

