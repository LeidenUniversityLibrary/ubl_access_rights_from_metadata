<?php

/**
 * @file
 * ubl-detail-tools.tpl.php
 *
 * @TODO: needs documentation about file and variables
 * $class - the class of the detail tools
 * $buttons - array of buttons
 *    array(
 *      'url' => URL of the button,
 *      'label' => label of the button,
 *      'class' => classes for the button,
 *      'additional' => html that is included within the LI of the button
 *    )
 */

?>

 <?php if (count($buttons) > 0): ?>
   <ul class="dc-detail-tools <?php print(is_array($class)?implode(' ', $class):$class); ?>">
   <?php foreach ($buttons as $button): ?>
     <li>
       <?php print l('<span>'.$button['label'].'</span><i class="fa '.$button['class'].'" aria-hidden="true"></i>', 
                     $button['url'],
                     array('attributes' => array('title' => $button['label']),'html' => TRUE)); ?>
     </li>
     <?php if (isset($button['additional'])): ?>
     <div class="additional">
       <?php print $button['additional']; ?>
     </div>
     <?php endif; ?>
   <?php endforeach; ?>
   </ul>
 <?php endif; ?>
