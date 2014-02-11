<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 5:26 PM
 */

class FieldFormResultController extends EntityAPIController {
  public function __construct($entityType) {
    parent::__construct('fieldform_result');
  }

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $entity) {
      if (!isset($entity->module)) {
        $entity->module = 'fieldform';
      }
      if (!empty($entity->data)) {
        $data = (array) $entity->data;
        $tmp = array();
        foreach ($data as $name => $value) {
          $type = is_array($value) ? $value['type'] : $value->type;
          $tmp[$name] = fieldform_result_create($type, $value);
        }
        $entity->data = $tmp;
      }
    }
    reset($entities);
    return $entities;
  }

  public function save($entity) {
    if (isset($entity->is_new)) {
      $entity->created = REQUEST_TIME;
    }
    $entity->changed = REQUEST_TIME;

    if (!isset($entity->module)) {
      $entity->module = 'fieldform';
    }

    $data = $entity->data;
    if (!empty($data)) {
      $tmp = array();
      foreach ($data as $name => $value) {
        $tmp[$name] = $value->save();
      }
      $entity->data = $tmp;
    }

    parent::save($entity);
    $entity->data = $data;
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $data = array();

    foreach ($entity->data as $name => $value) {
      $data[$name] = $value->render($view_mode);
    }

    return $data;
  }

  static public function user(FieldFormResultEntity $entity) {
    return user_load($entity->uid);
  }

  static public function add(FieldFormResultEntity $entity, $data) {
    if (is_array($data) && isset($data['name']) && isset($data['type'])) {
      $data = fieldform_result_create($data['type'], $data);
    }

    if (isset($data->name)) {
      $entity->data[$data->name] = $data;
    }
  }

  static public function update(FieldFormResultEntity $entity, $data) {
    if (is_array($data) && isset($data['name']) && isset($data['type'])) {
      $data = fieldform_result_create($data['type'], $data);
    }

    if (!empty($entity->data[$data->name])) {
      $entity->data[$data->name] = $data;
      return $entity;
    }
    return FALSE;
  }

  static public function reset(FieldFormResultEntity $entity) {
    $entity->data = array();
  }

  static public function buildFormElement(FieldFormResultEntity $entity, FieldFormElementInterface $element,
                                          &$form, &$form_state) {
    if (is_array($element->parents)) {
      $child = &drupal_array_get_nested_value($form, $element->parents);
      $default_value = isset($child['#parents']) && is_array($child['#parents']) ?
        drupal_array_get_nested_value($form_state, $child['#parents']) : NULL;
    }
    elseif (isset($form[$element->name])) {
      $child = &$form[$element->name];
      $default_value = isset($child['#parents']) && is_array($child['#parents']) ?
        drupal_array_get_nested_value($form_state, $child['#parents']) : NULL;
    }

    if (isset($entity->data[$element->name]) && is_array($child)) {
      $result = $entity->data[$element->name];
      $result->buildElement($child, $default_value);
    }
    if ($element->hasChildren() && is_array($child)) {
      foreach($element->children as $element_child) {
        self::buildFormElement($entity, $element_child, $child, $form_state);
      }
    }
  }

  static public function buildForm(FieldFormResultEntity $entity, FieldFormEntity $fieldform, &$form, &$form_state) {
    if (!empty($entity->data)) {
      $separator = $fieldform->getSeparatorId($form_state);
      $elements = $fieldform->getSection($separator, $form_state);
      foreach ($elements as $element) {
        self::buildFormElement($entity, $element, $form, $form_state);
      }
    }
  }

  static public function getSection(FieldFormResultEntity $entity, FieldFormEntity $fieldform,
                                    $separator = NULL) {
    /**
     * The separator, in this case, is the to initiate the next section. But here we need to get the
     * previous results. So we need to use the separator as an end parameter and use it to determine the start
     */

    $separators = array_keys($fieldform->findElementsByType('separator'));
    $iterator = new ArrayIterator($fieldform->elements);
    $iterator->rewind();
    $ret = array();

    if (!empty($separators)) {
      if (isset($separator)) {
        $end = array_search($separator, $separators);
        // Get the previous separator, if exists.
        if (($end--) > 0) {
          $separator = $separators[$end];
          $seek = array_search($separator, array_keys($fieldform->elements));
          $iterator->seek($seek);

          if ($iterator->valid()) {
            $iterator->next();
          }
        }
      }
    }

    while($iterator->valid() && $iterator->current()->elementType() !== 'separator') {
      $ret[] = $iterator->current();
      $iterator->next();
    }
    return $ret;
  }

  static public function processResultElement(FieldFormResultEntity $entity, FieldFormElementInterface $element,
                                              $form_state) {
    $name = $element->name;
    $value = drupal_array_get_nested_value($form_state['values'], array($name));
    if (isset($value)) {
      $data = array(
        'name' => $name,
        'type' => $element->elementType(),
        'value' => $value,
        'title' => $element->title
      );
      $result = fieldform_result_create($element->elementType(), $data);
      $value = $result->processResult($entity, $element, $value);
      $result->value($value);
      if ($entity->update($result) === FALSE) {
        $entity->add($result);
      }

      if ($element->hasChildren()) {
        foreach ($element->children as $element_child) {
          self::processResultElement($entity, $element_child, $form_state);
        }
      }
    }
  }

  static public function processResults(FieldFormResultEntity $entity, FieldFormEntity $fieldform, $form_state) {
    $separator = $fieldform->getSeparatorId($form_state);
    $separator = $fieldform->getPreviousSeparatorId($separator);
    $form_state['input']['separator'] = $separator;
    $elements = $fieldform->getSection($separator, $form_state);
    $form = $fieldform->buildForm(array(), $form_state);
    dpm($form, 'process Results');
    foreach ($elements as $element) {
      self::processResultElement($entity, $element, $form_state);
    }
  }

  static public function buildResult(FieldFormResultEntity $entity, FieldFormEntity $fieldform, $form_state) {
    $entity->form_name = $fieldform->identifier();
    if (!isset($entity->uid)) {
      $entity->uid = $GLOBALS['user']->uid;
      $entity->hostname = $GLOBALS['user']->hostname;
    }
    self::processResults($entity, $fieldform, $form_state);
    dpm($entity, 'Result');
    return $entity;
  }
}

interface FieldFormResultDataControllerInterface {
  public function resultType();
  public function resultInfo();
  public function create($resultType, $values = array());
  static public function data(FieldFormResultDataInterface $result);
  static public function value(FieldFormResultDataInterface $result, $set = NULL);
  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full');
  static public function save(FieldFormResultDataInterface $result);
  static public function buildElement(FieldFormResultDataInterface $result, &$element);
  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value);
}

class FieldFormResultDataController implements FieldFormResultDataControllerInterface {
  protected $resultType;
  protected $resultInfo;

  public function __construct($resultType) {
    $this->resultType = $resultType;
    if ($this->resultInfo = fieldform_result_get_info($resultType) === FALSE) {
      $this->resultInfo = fieldform_result_get_info('default');
    }
  }

  public function create($resultType, $values = array()) {
    $class = isset($this->resultInfo['result class']) ? $this->resultInfo['result class'] : 'FieldFormResultData';
    return new $class($resultType, $values);
  }

  public function resultType() {
    return $this->resultType;
  }

  public function resultInfo() {
    return $this->resultInfo;
  }

  static public function data(FieldFormResultDataInterface $result) {
    $ret = get_object_vars($result);
    return $ret;
  }

  static public function value(FieldFormResultDataInterface $result, $set = NULL) {
    return $result->value($set);
  }

  static public function render(FieldFormResultDataInterface $result, $view_mode = 'full') {
    return array(
      '#title' => $result->title,
      '#result' => $result,
      '#theme' => 'fieldform_result'
    );
  }

  static public function save(FieldFormResultDataInterface $result) {
    $data = self::data($result);
    $data['value'] = $result->value();
    return $data;
  }

  static public function buildElement(FieldFormResultDataInterface $result, &$element, $default_value = NULL) {
    $value = isset($default_value) ? $default_value : $result->value();

    if (isset($value)
      && ((isset($element['#default_value']) && $element['#default_value'] !== $value)
      || (!isset($element['#default_value'])
        && !in_array($element['#type'], array('hidden', 'fieldset', 'token', 'button'))))) {
      $element['#default_value'] = $value;
    }
    elseif (isset($value) && in_array($element['#type'], array('hidden', 'token'))
      && isset($element['#default_value'])
      && ((isset($element['#value']) && $element['#default_value'] !== $value)
        || !isset($element['#value']))) {
      $element['#value'] = $value;
    }
  }

  static public function processResult(FieldFormResultDataInterface $result, FieldFormResultEntity $entity,
                                       FieldFormElementInterface $element, $value) {
    return $value;
  }

  static public function preRender(FieldFormResultDataInterface $result, &$vars) {
    $value = $result->value();
    if (is_array($value)) {
      $items = array('items' => $value);
      if ($vars['title_display'] == 'inline') {
        $items =+ array(
          'class' => 'inline'
        );
      }
      $vars['content'] = theme('item_list', $items);
    }
    else {
      $vars['content'] = $value;
    }
  }
}