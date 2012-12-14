<?php
/**
 * @file
 * Template file for ding_list
 *
 * Variables 
 *
 */
if (isset($no_result)) {
  print '<div class="no-result">';
  foreach ($no_result as $operation) {
    print '<div class="operation"';
    foreach ($operation as $key => $value) {
      switch ($key) {
        case 'key_val':
          print ' data-key="' . $value[0] . '" data-value="' . $value[1] . '"';
        break;
        case 'condition':
          print ' data-condition="' . $value[0] . '" data-condition-value="' . $value[1] . '"';
        break;
      }
    }
    print '></div>';
  }
  print '</div>';
}
?>
<div id='<?php print $divid; ?>' class="<?php print implode($classes_array," ");?>">
  <?php if(!empty($prefix)) { print $prefix; } ?>
  <div class="list-content">
    <?php print $content; ?>
  </div>
  <?php if(!empty($pager)) { print render($pager); } ?>
  <?php if(!empty($suffix)) { print $suffix; } ?>
</div>