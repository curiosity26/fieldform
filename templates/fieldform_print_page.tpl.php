<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/3/14
 * Time: 3:46 PM
 */
?>
<div id="wrapper" <?php print $attributes ?> class="<?php print $classes ?>">
  <div id="page-header">
    <?php print $logo ?>
    <h1><?php print $title ?></h1>
  </div>
  <?php print render($results) ?>
</div>