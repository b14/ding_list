/*jshint forin:false, jquery:true, browser:true, indent:2, trailing:true, unused:false */
/*global Drupal */

(function ($) {
  "use strict";

  // -----------------------------------------------------------------------------------------------
  // Call initialize
  Drupal.behaviors.ding_list_sorting = {
    // Notice that we don't use context, because we want to process the whole page every time.
    attach: function (context, settings) {
      $('.ding-list-list.sort-enabled > .ding-list-items', context).sortable({
        handle: '.media, .knot',
        distance: 20,
        stop: function (e, ui) {
          var
            order = [],
            list_id = $(this).closest('.ding-list-list').attr('ref'),
            $item = $(ui.item);

          $(this).find('.entity').each(function (i, element) {
            order.push($(element).attr('ref'));
          });

          $.ajax({
            url: Drupal.settings.basePath + 'dinglist/set_order/' + list_id,
            type: 'POST',
            dataType: 'json',
            data: {'order' : order, 'item': $item.attr('ref'), 'previous': $item.prev().attr('ref')},
            success: function (data, textStatus, XMLHttpRequest) {  }
          });
        }
      });
    }
  };
}(jQuery));