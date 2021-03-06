<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/29/14
 * Time: 4:39 PM
 */
?>
<div <?php print $attributes ?> class="<?php print $classes ?>">
  <?php if (isset($title)): ?>
    <h3 <?php print $title_attributes ?>>
      <?php print render($title_prefix).$title.render($title_suffix) ?>
    </h3>
  <?php endif; ?>
  <div <?php print $content_attributes?>>
    <?php print $content ?>
  </div>
</div>