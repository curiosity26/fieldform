<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 2/10/14
 * Time: 1:42 PM
 */

class SingleOptionResultDataController extends FieldFormResultDataController {
  static public function preRender(FieldFormResultDataInterface $result, &$vars) {

    $vars['title'] = NULL;
    $vars['content'] = ($result->value() == TRUE ? '&#9635;' : '&#9634;').$result->title;
  }
}