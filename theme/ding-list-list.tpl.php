<div <?php if ($sortable !== FALSE) { print 'ref="' . $sortable . '" '; } ?>class="<?php print $classes; ?>">
  <div class="ding-list-items">
    <?php print render($items); ?>
  </div>
</div>