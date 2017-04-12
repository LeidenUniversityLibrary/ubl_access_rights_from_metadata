# UBL access rights from metadata

## Introduction

UBL Access rights from metadata changes the XACML policy based on the metadata of the object. It also defines which datastreams are downloadable.

## How to include download buttons

```php
// Render the detail tools block
 $block = module_invoke_all('detail_tools_block_view');
 if (isset($block['content'])) {
   print render($block['content']);
 }
```
