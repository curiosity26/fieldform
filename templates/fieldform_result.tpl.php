<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/29/14
 * Time: 4:39 PM
 */
?>
<div <?php print $attributes ?> class="<?php print $classes ?>">
  <<?php if($title_display === 'before'):print 'span';else:print 'div';endif;?> <?php print $title_attributes ?>>
    <?php print render($title_prefix).$title.render($title_suffix) ?>
  </<?php if($title_display === 'before'):print 'span';else:print 'div';endif;?>>
  <<?php if($title_display === 'before'):print 'span';else:print 'div';endif;?> <?php print $content_attributes?>>
    <?php print $content ?>
  </<?php if($title_display === 'before'):print 'span';else:print 'div';endif;?>>
</div>