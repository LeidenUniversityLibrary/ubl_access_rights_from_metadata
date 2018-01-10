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

The configuration file should have the following parts:

The values of the "`*_to_dsid`" and "`provide_download_of_dsid`" keys should be valid DSID's, or ALL (for all datastreams) or NONE (for no datastream at all).

Below examples of the parts of the configuration file:

### Metadata values

This part of the configuration is meant to define the metadata values that are used in other parts of the configuration.
This part contains zero or more sections. Each section is named "metadata:key" where "key" is a chosen value.
A "metadata:" section can contain the following keys:

 * datastream: the id of the datastream where the metadata is extracted from (mandatory)
 * xpath: a xpath to extract the data from the datastream (mandatory)
 * namespace: the namespace definition, use like "mods http://www.loc.gov/mods/v3" (multiple values, optional)

### Access conditions

This part of the configuration is meant to define the conditions.
This part should contain 1 or more sections. Each section is named "condition:key" where "key" is a chosen value.

A "condition:" section can contain the following keys:
 * `user_role` (multiple values, optional)
 * `ip_range` (multiple values, optional)
 * condition (multiple values, optional)
 * operator (optional)
 * metadata (optional)
 * comparator (optional)
 * value (optional)

### Access restrictions

This part of the configuration is meant to define the access restrictions for the conditions that are met.
This part contains 1 or more sections. Each section is named "access:`condition_key`" where "`condition_key`" is defined in the "access conditions" part of the configuration.

A "access:" section can contain the following keys:
 * `deny_viewing`: TRUE or FALSE, defines is the whole object can be viewed
 * `deny_access_to_dsid` (multiple values, optional)
 * `allow_access_to_dsid` (multiple values, optional)
 * `provide_download_of_dsid` (multiple values, optional)
 * `access_text`' (optional)
 * `access_usetext`' (optional)
 * `access_link`' (optional)
 * `access_image`' (optional)

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

### check\_access\_rights\_settings

Check the access rights value of the children of a specific collection.

```
drush --user=admin check_access_rights_settings --collection=islandora:root --configuration=/path/to/a/ini/file
drush --user=admin check_access_rights_settings --collection=islandora:root --configuration=/url/to/a/ini/file
drush --user=admin cars --collection=islandora:root --configuration=/url/to/a/ini/file
```


## Maintainers/Sponsors

Current maintainers:

* [Lucas van Schaik](https://github.com/lucasvanschaik)

## Development

If you would like to contribute to this module, please contact the current maintainer.


## License

[GPLv3](LICENSE.txt)
Copyright 2017 Leiden University Library

