if (!Function.prototype.bind) {
  Function.prototype.bind = function (oThis) {
    if (typeof this !== "function") {
      // closest thing possible to the ECMAScript 5 internal IsCallable function
      throw new TypeError("Function.prototype.bind - what is trying to be bound is not callable");
    }
 
    var aArgs = Array.prototype.slice.call(arguments, 1), 
        fToBind = this, 
        fNOP = function () {},
        fBound = function () {
          return fToBind.apply(this instanceof fNOP && oThis
                                 ? this
                                 : oThis,
                               aArgs.concat(Array.prototype.slice.call(arguments)));
        };
 
    fNOP.prototype = this.prototype;
    fBound.prototype = new fNOP();
 
    return fBound;
  };
}

/* ----------------------------------------------------------------------------------------------- */

(function ($) {
  	
  Drupal.behaviors.ding_ting = {
    attach: function(context, settings) {
	    //
	    // Initialize ding lists
	    //
      
      $('.ding-list', context).each(function (i, list) {
        var $list = $(list);
        
        $('.pagination .change-view', $list).click(function() {
          $('.change-view', $(this).closest('.item-list')).removeClass('active');
          $(this).addClass('active');
        });
        $('.pagination', $list).find('.change-view:first').addClass('active');
        
        var tabs = $('.tabs', $list).each(function() {
          $('li', this).first().addClass('tab-active');
        });
        
        if (tabs.length == 0) {
          var
            _tabId = $('.tabs-content', $list).children().first().attr('id');
            if (_tabId === undefined) {
              return;
            }
            var listId = _tabId.replace('-content', '');
          
          // Setup view mode changes
          $('.pagination .change-view', $list).each(function() {
            // Set variables.
            var $href = $(this).attr('href'),
              $split = $(this).attr('href').split('?'),
              $vars = $split[1].split('&');
            
            // Change href.
            $href = $split[0];
            $href += '?html_id=' + _tabId;
            $href += '&tab=' + listId;
            $href += '&' + $vars.pop();
            if(get_data('debug') == 's') {
              $href += '&' + 'debug=s';
            }
            $(this).attr('href', $href);
            
            // Set ajax variables to reflect the new url.
            Drupal.ajax[$(this).attr('id')].options.url = $href;
            Drupal.ajax[$(this).attr('id')].submit.ding_list = Drupal.settings.ding_list[listId];
          }).first().click();
          return;
        }
        
        $('a', tabs).each(function() {
          $tabId = $(this).attr('id');
          if(Drupal.ajax[$tabId]) {
            Drupal.ajax[$tabId].submit.ding_list = Drupal.settings.ding_list[$tabId];
            Drupal.ajax[$tabId].success = tabEventResponse;
          }
        });
        $('.view-more', $list).each(function() {
          $tabId = $(this).attr('id');
          if(Drupal.ajax[$tabId] && Drupal.settings.ding_list[$tabId]) {
            Drupal.ajax[$tabId].submit.ding_list = Drupal.settings.ding_list[$tabId];
            Drupal.ajax[$tabId].success = tabEventResponse;
          }
        });
        // Clicks on anchor tabs
        $('a', tabs).click(function() {
          var _tabId = $(this).attr('id');
          
          if(!$(this).hasClass('use-ajax')) {
            // switch tabs
            var $tabId = $(this).attr('id');
            var $contentId = '#' + $tabId + '-content';
            // Hide other content and show relevant
            $(this).closest('.ding-list').find('.ding-list-content').addClass('tab-hidden');
            $($contentId).removeClass('tab-hidden');
            
            // Mark active tab as well
            $(this).closest('.tabs').find('li').removeClass('tab-active');
            $(this).closest('li').addClass('tab-active');
          }
          
          // Setup view mode changes
          var $list = $(this).closest('.ding-list');
          $('.pagination .change-view', $list).each(function() {
            // Set variables.
            var $href = $(this).attr('href'),
              $split = $(this).attr('href').split('?'),
              $vars = $split[1].split('&');
            
            // Change href.
            $href = $split[0];
            $href += '?html_id=' + _tabId + '-content';
            $href += '&tab=' + _tabId;
            $href += '&' + $vars.pop();
            if(get_data('debug') == 's') {
              $href += '&' + 'debug=s';
            }
            $(this).attr('href', $href);
            
            // Set ajax variables to reflect the new url.
            Drupal.ajax[$(this).attr('id')].options.url = $href;
            Drupal.ajax[$(this).attr('id')].submit.ding_list = Drupal.settings.ding_list[_tabId];
          });
        });
      });
      
      
      // DELETE THE REST OF THE FUNCTION IF THE ABOVE WORK PROPERLY!
      return;
      
	    // Add tab-active to first item
	    var tabs = $('.ding-list .tabs', context).each(function() {
  	    $('li', this).first().addClass('tab-active');
	    });
      
      if (tabs.length == 0) {
        var
          $list = $('.ding-list', context),
          $first_content = $('.tabs-content').children().first(),
          _tabId = $first_content.attr('id');
        
        // Setup view mode changes
        $('.pagination .change-view', $list).each(function() {
          // Set variables.
          var $href = $(this).attr('href'),
            $split = $(this).attr('href').split('?'),
            $vars = $split[1].split('&');
          
          // Change href.
          $href = $split[0];
          $href += '?html_id=' + _tabId;
          $href += '&tab=' + _tabId.replace('-content', '');
          $href += '&' + $vars.pop();
          $(this).attr('href', $href);
          
          // Set ajax variables to reflect the new url.
          Drupal.ajax[$(this).attr('id')].options.url = $href;
          Drupal.ajax[$(this).attr('id')].submit.ding_list = Drupal.settings.ding_list[_tabId.replace('-content', '')];
        });
      }
      
      $('a', tabs).each(function() {
        $tabId = $(this).attr('id');
        if(Drupal.ajax[$tabId]) {
          Drupal.ajax[$tabId].submit.ding_list = Drupal.settings.ding_list[$tabId];
          Drupal.ajax[$tabId].success = tabEventResponse;
        }
      });
	    // Clicks on anchor tabs
	    $('a', tabs).click(function() {
	      var _tabId = $(this).attr('id');
	      
	      if(!$(this).hasClass('use-ajax')) {
          // switch tabs
          var $tabId = $(this).attr('id');
          var $contentId = '#' + $tabId + '-content';
          // Hide other content and show relevant
          $(this).closest('.ding-list').find('.ding-list-content').addClass('tab-hidden');
          $($contentId).removeClass('tab-hidden');
          
          // Mark active tab as well
          $(this).closest('.tabs').find('li').removeClass('tab-active');
          $(this).closest('li').addClass('tab-active');
	      }
	      
        // Setup view mode changes
        var $list = $(this).closest('.ding-list');
        $('.pagination .change-view', $list).each(function() {
          // Set variables.
          var $href = $(this).attr('href'),
            $split = $(this).attr('href').split('?'),
            $vars = $split[1].split('&');
          
          // Change href.
          $href = $split[0];
          $href += '?html_id=' + _tabId + '-content';
          $href += '&tab=' + _tabId;
          $href += '&' + $vars.pop();
          $(this).attr('href', $href);
          
          // Set ajax variables to reflect the new url.
          Drupal.ajax[$(this).attr('id')].options.url = $href;
          Drupal.ajax[$(this).attr('id')].submit.ding_list = Drupal.settings.ding_list[_tabId];
        });
	    });
	    $('.ding-list .pagination .change-view', context).click(function() {
	      $('.change-view', $(this).closest('.item-list')).removeClass('active');
	      $(this).addClass('active');
	    });
	    $('.ding-list .pagination', context).find('.change-view:first').addClass('active');
    }
  }
  
  $(document).ready(function() {
    if(location.hash) {
      var $id = location.hash.replace('tab-', '');
      if($($id).size() > 0) {
        $($id).click();
      }
    }
    $('.ding-list .tabs li.tab-active').each(function() {
      $(this).children('a').click();
    });
    $('.ding-list .tabs a').click(function() {
      location.hash = 'tab-' + $(this).attr('id');
    });
  });
  
  // We only need to look for deeplinks when the page is loaded, and not when any
  // drupal behavior is called.
  $(function () {
    setTimeout(function () {
      if (location.hash != '') {
        var link = $('.ding-list .tabs a[href="' + location.hash + '"]');
        
        if (link.length > 0) {
          link.click();
        }
      }
    }, 10);
  });

  /**
   * Attaches the behavior handling the action buttons.
   *
   * This Includes both setting up the actual action button, and the action
   * menu lists.
   */
  Drupal.behaviors.ding_list_handler = {
    attach: function (context, settings) {
      // Setup the list paging
      $('.ding-list-ajax-load', context).each(function (i, ding_list) {
        var
          $list = $(ding_list),

          $link = $list.find('.pager a').each(function() {
            $(this).click(function (e) {
              e.preventDefault();
            });
            if(Drupal.ajax[$(this).attr('id')])
              Drupal.ajax[$(this).attr('id')].eventResponse = listEventResponse;
          });
      });
      return;
    }
  };
  
  /**
   * This will handle the no results from the dinglist.
   */
  Drupal.behaviors.ding_list_no_result_handler = {
    attach: function (context, settings) {
      if (context == document) {
        return;
      }
      $no_result = $('.no-result .operation', context).each(function (i, operation) {
        var
          $operation = $(operation),
          key = $operation.data('key'),
          val = $operation.data('value'),
          condition = $operation.data('condition'),
          condition_val = $operation.data('condition-value'),
          $list = context.closest('.ding-list');
          
        switch (condition) {
          case 'not_exist':
            if ($(condition_val, $list).length > 0) {
              return;
            }
          break;
        }
        
        switch (key) {
          case 'hide':
            $(val, $list).hide();
          break;
        }
      });
      
    }
  };
  
  /**
   * A custom eventResponse, we use for our Drupal.AJAX objects.
   *
   * @todo Comments
   */
  function listEventResponse(element, event) {
    var ajax = this;

    // This is the only difference.
    // Do not perform another ajax command if one is already in progress.
    if (ajax.ajaxing || $(element).closest('.ding-list-ajax-load').hasClass('processed')) {
      return false;
    }

    try {
      if (ajax.form) {
        if (ajax.setClick) {
          element.form.clk = element;
        }

        ajax.form.ajaxSubmit(ajax.options);
      }
      else {
        ajax.beforeSerialize(ajax.element, ajax.options);
        $.ajax(ajax.options);
      }
    }
    catch (e) {
      ajax.ajaxing = false;
      alert("An error occurred while attempting to process " + ajax.options.url + ": " + e.message);
    }

    if (typeof element.type != 'undefined' && (element.type == 'checkbox' || element.type == 'radio')) {
      return true;
    }
    else {
      return false;
    }
  };
  
  /**
   * A custom eventResponse, we use for our Drupal.AJAX objects.
   *
   * @todo Comments
   */
  function tabEventResponse(response, status) {
    // Remove the progress element.
    if (this.progress.element) {
      $(this.progress.element).remove();
    }
    if (this.progress.object) {
      this.progress.object.stopMonitoring();
    }
    $(this.element).removeClass('progress-disabled').removeAttr('disabled');
  
    Drupal.freezeHeight();
  
    for (var i in response) {
      if (response[i]['command'] && this.commands[response[i]['command']]) {
        this.commands[response[i]['command']](this, response[i], status);
      }
    }
  
    // Reattach behaviors, if they were detached in beforeSerialize(). The
    // attachBehaviors() called on the new content from processing the response
    // commands is not sufficient, because behaviors from the entire form need
    // to be reattached.
    if (this.form) {
      var settings = this.settings || Drupal.settings;
      Drupal.attachBehaviors(this.form, settings);
    }
  
    Drupal.unfreezeHeight();
  
    // Remove any response-specific settings so they don't get used on the next
    // call by mistake.
    this.settings = null;
    
    // switch tabs
    var $element = this.element;
    var $tabId = $($element).attr('id');
    var $contentId = '#' + $tabId + '-content';
    // Hide other content and show relevant
    $($element).closest('.ding-list').find('.ding-list-content').addClass('tab-hidden');
    $($contentId).removeClass('tab-hidden');
    
    // Mark active tab as well
    $($element).closest('.tabs').find('li').removeClass('tab-active');
    $($element).closest('li').addClass('tab-active');
  };

  Drupal.behaviors.ding_list_mode_toggler = {
    attach: function(context, settings) {
      try {
        if(Drupal.settings.dingList) {
          var listTypes = Drupal.settings.dingList.listModes;
          var listTypeDefault = Drupal.settings.dingList.listTypeDefault;
        }
      } catch(err) {
        
      }
      
      $('.ding-list-list, .pane-main-main-ting-object-types').each(function() {
        var $list = $('.ting-object', this);
        $list.addClass(listTypeDefault);

        $('.ding-list-mode-toggler li a', this).click(function (e) {
          e.preventDefault();  
          // What list type has the user clicked on?
          var currentListType = $(this).attr('id');
          
          $list.removeClass(listTypes.join(' '));
          $list.addClass(currentListType);
        });
      });
    }
  }
  
  /*
   * object: Ribbon
   *
   * Object to handler colored ribbon sizing
   * (the ones with section titles in them).
   * 
   * They're supposed to span across the entire 
   * screen estate. Because of Panels, and the way
   * the HTML is currently structured, this is not
   * possible without restructuring THE ENTIRE SITE.
   *
   * The object makes sure that the ribbons always
   * spans the entire width, using only one resize
   * callback.
   *
   * /AFK
   */
  
  var ribbon = {
    elements: [],
    handlerIsAttached: false,
    add: function(elem) {
      this.elements.push(elem);
      if (!this.handlerIsAttached) {      
        $(window).resize(this.resize.bind(this));
        this.handlerIsAttached = true;
      } 
      $(window).trigger('resize');
    },
    resize: function(e) {
      var win = $(window);
      $.each(this.elements, function(k, v) {
        var 
          elem = $(v)
          parent = elem.parent()
          margin = ($(parent).width() - win.width()) / 2;
        elem.css({
          marginLeft: margin,
          marginRight: margin
        });
      });
    }
  }
  
  // Check for get variable.
  function get_data(name){
    name = name.replace(/[\[]/,"\\\[").replace(/[\]]/,"\\\]");
    var regexS = "[\\?&]"+name+"=([^&#]*)";
    var regex = new RegExp(regexS);
    var results = regex.exec(window.location.href);
    if(results == null) return "";
    else return results[1];
  }
  
  Drupal.behaviors.ding_list_ribbon = {
    attach: function(context, settings) {
      // We use vanilla css instead
      //$('.ding-list .section-header', context).each(function() {
      //  ribbon.add(this);
      //});
    }
  }
})(jQuery);