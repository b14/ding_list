<?php
/**
 * @file
 * Template file for ding_list
 *
 * Variables 
 * $from_preprocess 
 *
 * Section header has an 'inner' div aswell as a 
 * wrapper inside that. This is required unfortunately, to
 * accommodate the design..
 * 
 * /AFK  
 */
 
// No tabs No output
if (!isset($tabs)) {
  print "<!-- NO CONTENT: $title -->"; 
  return;
}
?>
<section class="<?php print $classes;?>"<?php print drupal_attributes($list_attributes); ?>>
  <header class="section-header">
    <div class="inner">
      <div class="wrapper">
        <h2 class="title"><?php print $title; ?></h2>
        <div class="pagination">
          <div class="inner">
            <?php print $navigation;?>
          </div>
        </div>
      </div>
    </div>
  </header>
  
  <div class="content">
    <?php if (isset($tabs['links']) && count($tabs['links']) > 0) { ?>
    <div class="navigation">
      <div class="inner">
        <div class="more"><?php print $more;?></div>
        <div class="tabs">
          <?php print render($tabs['links'])?>
        </div>
      </div>
    </div>
    <?php } ?>
    <div class="tabs-content">
      <?php  print render($tabs); ?>
    </div>
  </div>
</section>
