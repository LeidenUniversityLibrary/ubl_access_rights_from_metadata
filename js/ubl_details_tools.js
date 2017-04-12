/**
 * @file
 * js/ubl_details_tools.js
 */

jQuery(document).ready(function() {
  jQuery('.ubl-detail-tools LI A .fa-download').parent().click(function (e) {
    var $li = jQuery(this).parent();
    var $additionalblock = $li.next('.additional'); 
    if ($additionalblock.size() > 0) {
      e.preventDefault();
      if ($additionalblock.is(':visible')) {
        return; // is already visible
      }
      $additionalblock.find('TR').each(function() {
        var downloadurl = jQuery(this).data('download');
        jQuery(this).click(function (e) {
          document.location = downloadurl;
        });
      });
      var x = $li.offset().left - $additionalblock.outerWidth() + $li.outerWidth();
      if (x > 0) {
        $additionalblock.css('left', x + 'px');
      }
      $additionalblock.slideDown();
      e.stopPropagation();
      jQuery('BODY').one('click', function() {
        $additionalblock.slideUp();
      });
    }
  });
});
