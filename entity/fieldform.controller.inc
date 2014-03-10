<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 1/16/14
 * Time: 2:34 PM
 */

class FieldFormController extends EntityAPIControllerExportable {
  public function __construct($entityType = NULL) {
    parent::__construct('fieldform');
  }

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    foreach ($entities as $entity) {
      if (!empty($entity->elements) && is_array($entity->elements)) {
        $elements = array();
        foreach ($entity->elements as $value) {
          if (is_string($value)) {
            $value = unserialize($value);
          }
          $element = is_array($value) ? fieldform_element_create($value['type'], $value) : $value;
          $elements[$element->_id] = $element;
        }
        $entity->elements = $elements;
        dpm($entity->elements, 'Wakeup');
        if (!$entity->hasElement('form_complete')) {
          $terminator = fieldform_element_create('separator', array(
            '_id' => 'form_complete',
            'name' => 'form_complete',
            'title' => 'Form Terminator',
            'weight' => 100000
          ));
          $entity->addElement($terminator);
          entity_save($entity->entityType(), $entity);
        }
      }

      if (!isset($entity->settings) || !is_array($entity->settings)) {
        $entity->settings = array();
      }

      $entity->settings += array(
        'completed_behavior' => 'message',
        'completed_behavior_message' => array('value' => '', 'format' => NULL),
        'completed_behavior_url' => '',
        'submit_button_text' => t('Submit'),
        'continue_button_text' => t('Continue'),
        'back_button_text' => t('Back')
      );
    }

    return $entities;
  }

  public function create(array $values = array()) {
    $values += array('is_new' => TRUE);
    if (isset($this->entityInfo['entity class']) && $class = $this->entityInfo['entity class']) {
      $entity = new $class($values, $this->entityType);
      $elements = array();
      foreach ($entity->elements as $element) {
        if (is_string($element)) {
          $element = unserialize($element);
        }
        $element = is_array($element) ? fieldform_element_create($element['type'], $element) : $element;
        $elements[$element->_id] = $element;
      }
      $entity->elements = $elements;

      $entity->settings += array(
        'completed_behavior' => 'message',
        'completed_behavior_message' => array('value' => '', 'format' => NULL),
        'completed_behavior_url' => '',
        'submit_button_text' => t('Submit'),
        'continue_button_text' => t('Continue'),
        'back_button_text' => t('Back')
      );
    }

    return $entity;
  }

  public function export($entity, $prefix = '') {
    $vars = get_object_vars($entity);
    unset($vars[$this->statusKey], $vars[$this->moduleKey], $vars['is_new']);
    if ($this->nameKey != $this->idKey) {
      unset($vars[$this->idKey]);
    }

    $elements = array();

    foreach ($entity->elements as $child) {
      if ($child->name !== 'form_complete') {
        if (is_array($child)) {
          $elements[] = $child;
        }
        else {
          $elements[] = $child->toArray();
        }
      }
    }
    $vars['elements'] = $elements;

    return entity_var_json_export($vars, $prefix);
  }

  public function import($export) {
    $entity = parent::import($export);
    if (!!$entity) {
      $elements = array();
      foreach ($entity->elements as $item) {
        if (is_string($item)) {
          $item = unserialize($item);
        }
        $element = is_array($item) ? fieldform_element_create($item['type'], $item) : $item;
        $elements[$element->_id] = $element;
      }
      $entity->elements = $elements;
    }
    return $entity;
  }

  static public function hookForms() {
    $forms = array();
    $entities = fieldform_load_multiple();

    foreach ($entities as $entity) {
      $forms['fieldform_'.$entity->identifier().'_form'] = array(
        'callback' => 'fieldform_view_form',
      );
    }

    return $forms;
  }

  static public function sortWeights($a, $b) {
    $a_weight = (is_object($a) && isset($a->weight)) ? $a->weight : 0;
    $b_weight = (is_object($b) && isset($b->weight)) ? $b->weight : 0;
    if ($a_weight == $b_weight) {
      return 0;
    }
    return ($a_weight < $b_weight) ? -1 : 1;
  }

  static public function addElement(FieldFormEntity $entity, $element) {
    if (is_array($element)) {
      $element = fieldform_element_create($element['type'], $element);
    }

    if (empty($entity->elements) || empty($element->parents) || count($element->parents) == 1) {
      if ($element->_id) {
        $entity->elements[$element->_id] = $element;
        uasort($entity->elements, 'FieldFormController::sortWeights');
        return TRUE;
      }
      return FALSE;
    }

    reset($entity->elements);
    while (($object = current($entity->elements))) {
      if (($object->addElement($element)) !== FALSE) {
        return TRUE;
      }
      next($entity->elements);
    }
    return FALSE;
  }

  static public function updateElement(FieldFormEntity $entity, FieldFormElement $element) {
    if (($old = $entity->findElementByID($element->_id))) {
      $old->updateElement($element->toArray());
    }
  }

  static public function deleteElement(FieldFormEntity $entity, FieldFormElement $element) {
    if (empty($entity->elements)) {
      return FALSE;
    }
    dpm($entity->elements, 'Elements');
    dpm($element, 'Element');
    if (array_key_exists($element->_id, $entity->elements)) {
      unset($entity->elements[$element->_id]);
      return TRUE;
    }

    foreach ($entity->elements as &$child) {
      if (is_string($child)) {
        $child = unserialize($child);
      }
      if ($child->deleteElement($element) === TRUE) {
        return TRUE;
      }
    }

    return FALSE;
  }

  static public function hasElement(FieldFormEntity $entity, $id) {
    if (empty($entity->elements)) {
      return FALSE;
    }
    if (in_array($id, array_keys($entity->elements))) {
      return TRUE;
    }
    reset($entity->elements);
    while (($element = &current($entity->elements))) {
      if (is_string($element)) {
        $element = unserialize($element);
      }
      if (($element->hasChild($id)) !== FALSE) {
        return TRUE;
      }
      next($entity->elements);
    }
    return FALSE;
  }

  static public function findElementByID(FieldFormEntity $entity, $id) {
    if (empty($entity->elements)) {
      return FALSE;
    }

    if (in_array($id, array_keys($entity->elements))) {
      return $entity->elements[$id];
    }

    reset($entity->elements);
    foreach ($entity->elements as &$element) {
      if (is_string($element)) {
        $element = unserialize($element);
      }
      if (($found = $element->findChildByID($id)) !== FALSE) {
        return $found;
      }
    }

    return FALSE;
  }

  static public function findElementByName(FieldFormEntity $entity, $name) {
    if (empty($entity->elements)) {
      return FALSE;
    }
    reset($entity->elements);
    foreach ($entity->elements as &$element) {
      if (is_string($element)) {
        $element = unserialize($element);
      }
      if ($element->name === $name) {
        return $element;
      }

      if (($found = $element->findChildByName($name)) !== FALSE) {
        return $found;
      }
    }

    return FALSE;
  }

  static public function findElementsByType(FieldFormEntity $entity, $type) {
    $children = &drupal_static(__FUNCTION__, array());

    if (empty($children) || empty($children[$type])) {
      if (empty($entity->elements)) {
        return $children;
      }

      reset($entity->elements);
      $children[$type] = array();
      foreach ($entity->elements as &$element) {
        if (is_string($element)) {
          $element = unserialize($element);
        }
        if ($element->elementType() === $type) {
          $children[$type][$element->_id] = $element;
        }

        $found = $element->findChildrenByType($type);
        $children[$type] += $found;
      }
    }

    return $children[$type];
  }

  static public function getPreviousSeparatorId(FieldFormEntity $entity, $separator) {
    $separators = array_keys($entity->findElementsByType('separator'));
    $index = array_search($separator, $separators);
    if ($index > 0) {
      $index--;
      return $separators[$index];
    }
    return NULL;
  }

  static public function getSeparatorId(FieldFormEntity $entity, $form_state) {
    $separator = isset($form_state['values']['separator']) ?
      $entity->findElementByName($form_state['values']['separator']) : NULL;

    if (!isset($separator)) {
      $separator = isset($form_state['input']['separator']) ?
        $entity->findElementByName($form_state['input']['separator']) : NULL;
    }

    if (is_object($separator)) {
      return $separator->_id;
    }
    return NULL;
  }

  static public function getSectionIndex($entity, $separator) {
    return array_search($separator, array_keys($entity->elements));
  }

  static public function getSection($entity, $separator = NULL, &$form_state) {
    $separators = array_keys($entity->findElementsByType('separator'));
    $iterator = new ArrayIterator($entity->elements);
    $ret = array();

    if (!empty($separators) && count($separators) > 1) {
      if (isset($form_state['values']) && $form_state['values']['op'] === $entity->settings['back_button_text']) {
        $separator = self::getPreviousSeparatorId($entity, $separator); // The current page
        if (isset($separator)) {
          $separator = self::getPreviousSeparatorId($entity, $separator); // The previous page
        }
      }

      if (isset($separator)) {
        $seek = self::getSectionIndex($entity, $separator);
        $iterator->seek($seek);

        if ($iterator->valid()) {
          $iterator->next();
        }
      }
    }

    while($iterator->valid()) {
      $ret[] = $iterator->current();
      if ($iterator->current()->elementType() === 'separator') {
        break;
      }
      $iterator->next();
    }

    return $ret;
  }

  static public function processElements($entity, &$form, &$form_state) {
    $separator = self::getSeparatorId($entity, $form_state);
    // If the redirect option were chosen, it would already be redirected by now
    if (isset($form_state['storage']['complete'])) {
      $form['completed'] = array(
        '#markup' => $entity->settings['completed_behavior_message']['value']
      );
      return $form;
    }

    $elements = self::getSection($entity, $separator, $form_state);

    foreach ($elements as $element) {
      $form[$element->name] = $element->buildElement($entity, $form, $form_state);
    }

    $form['actions'] = array(
      '#type' => 'actions'
    );

    if (isset($separator)) {
      $form['actions']['back'] = array(
        '#type' => 'button',
        '#value' => $entity->settings['back_button_text']
      );
    }

    if (end($elements)->elementType() === 'separator' && end($elements)->name !== 'form_complete') {
      // If the last element processed was a form separator
      // Process the Continue button

      $form['actions']['continue'] = array(
        '#type' => 'submit',
        '#value' => $entity->settings['continue_button_text']
      );
    }
    else {
      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => $entity->settings['submit_button_text']
      );
    }

    return $form;
  }

  static public function buildForm($entity, $form, &$form_state) {
    $entity->processElements($form, $form_state);

    $form_state += array(
      'build_info' => array(
        'args' => array($entity->entityType(), $entity)
      ),
      'entity_type' => $entity->entityType()
    );

    $form_state[$entity->entityType()] = $entity;
    return $form;
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $form_state = array(
      'build_info' => array(
        'args' => array($entity->entityType(), $entity)
      )
    );
    return drupal_build_form('fieldform_'.$entity->identifier().'_form', $form_state);
  }

  static public function validate($entity, &$form, &$form_state) {
    $separator = self::getSeparatorId($entity, $form_state);
    $separator = self::getPreviousSeparatorId($entity, $separator);
    $elements = $entity->getSection($separator, $form_state);
    foreach ($elements as $element) {
      if (method_exists($element, 'validate')) {
        $element->validate($form, $form_state);
      }
    }
  }

  static public function submit($entity, $form, &$form_state) {
    $separator = self::getSeparatorId($entity, $form_state);
    $prev_separator = self::getPreviousSeparatorId($entity, $separator);
    $elements = $entity->getSection($prev_separator, $form_state);

    foreach ($elements as $element) {
      if (method_exists($element, 'submit')) {
        $element->submit($form, $form_state);
      }
    }

    if ($separator === 'form_complete') {
      $entity->completedFormBehavior($form, $form_state);
    }
    else {
      $form_state['rebuild'] = TRUE;
    }
  }

  static public function completedFormBehavior(FieldFormEntity $entity, $form, &$form_state) {
    if ($entity->settings['completed_behavior'] === 'message') {
      $form_state['rebuild'] = TRUE;
      $form_state['storage']['complete'] = TRUE;
    }
    else {
      $form_state['rebuild'] = FALSE;
      $form_state['redirect'] = url($entity->settings['completed_behavior_url']);
      drupal_redirect_form($form_state);
    }
  }

  public function save($entity) {
    $entity->changed = REQUEST_TIME;
    if (isset($entity->is_new) || !isset($entity->created)) {
      $entity->created = REQUEST_TIME;
    }
    $entity->module = isset($entity->entityInfo()['module']) ? $entity->entityInfo()['module'] : 'fieldform';

    // Ensure the terminator exists and is at the bottom always
    if (($terminator = $entity->findElementByName('form_complete')) === FALSE) {
      $terminator = fieldform_element_create('separator', array(
        '_id' => 'form_complete',
        'name' => 'form_complete',
        'title' => 'Form Terminator',
        'weight' => 100000
      ));
      $entity->addElement($terminator);
    }
    else {
      $clone = clone $terminator;
      $entity->deleteElement($terminator);
      $entity->addElement($clone);
    }
    $return = parent::save($entity);
    $this->resetCache(array($entity->identifier()));
    return $return;
  }
}

class FieldFormAdminUIController extends EntityDefaultUIController {
  public function hook_menu() {
    $menu = parent::hook_menu();
    $id_count = count(explode('/', $this->path));
    $wildcard = isset($this->entityInfo['admin ui']['menu wildcard']) ?
      $this->entityInfo['admin ui']['menu wildcard'] : '%entity_object';
    $element_wildcard = function_exists($this->entityType."_element_object_load") ?
      "%{$this->entityType}_element_object" : "%fieldform_element";
    // Add Elements Configuration

    $menu[$this->path."/manage/$wildcard/elements"] = array(
      'title' => 'Elements',
      'page callback' => 'drupal_get_form',
      'page arguments' => array('fieldform_elements_list', $id_count + 1),
      'access callback' => 'fieldform_access',
      'access arguments' => array('view', $id_count + 1),
      'type' => MENU_LOCAL_TASK
    );

    $menu[$this->path."/manage/$wildcard/elements/list"] = array(
      'title' => 'List',
      'type' => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10
    );

    $menu[$this->path."/manage/$wildcard/elements/$element_wildcard/edit"] = array(
      'title' => 'Edit Elements',
      'load arguments' => array($id_count + 1),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('edit', $id_count + 1, $id_count + 3),
      'access callback' => 'fieldform_access',
      'access arguments' => array('update', $id_count + 1),
      'type' => MENU_CALLBACK
    );

    $menu[$this->path."/manage/$wildcard/elements/$element_wildcard/delete"] = array(
      'title' => 'Delete Elements',
      'load arguments' => array($id_count + 1),
      'page callback' => 'fieldform_element_operations_form',
      'page arguments' => array('delete', $id_count + 1, $id_count + 3),
      'access callback' => 'fieldform_access',
      'access arguments' => array('delete', $id_count + 1),
      'type' => MENU_CALLBACK
    );

    if (isset($this->entityInfo['admin ui']['file'])) {
      // Add in the include file for the entity form.
      foreach (array("/manage/$wildcard/elements",
                 "/manage/$wildcard/elements/$element_wildcard/edit",
                 "/manage/$wildcard/elements/$element_wildcard/delete") as $path_end) {
        $menu[$this->path . $path_end]['file'] = $this->entityInfo['admin ui']['file'];
        $menu[$this->path . $path_end]['file path'] = isset($this->entityInfo['admin ui']['file path']) ?
          $this->entityInfo['admin ui']['file path'] : drupal_get_path('module', $this->entityInfo['module']);
      }
    }

    return $menu;
  }

  protected function operationCount() {
    return parent::operationCount() + 1;
  }

  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    $row = parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
    $second_half = array_splice($row, 2, count($row) - 3);
    $row[] = l(t('elements'), $this->path . '/manage/' . $id . '/elements');
    return array_merge($row, $second_half);
  }
}

interface FieldFormElementControllerInterface {
  static public function addElement(FieldFormElementInterface $element, $child);
  static public function updateElement(FieldFormElementInterface $element, array $values);
  static public function deleteElement(FieldFormElementInterface $element, FieldFormElementInterface $child);
  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity);
  static public function toArray(FieldFormElementInterface $element);
  static public function hasChild(FieldFormElementInterface $element, $id);
  static public function hasChildren(FieldFormElementInterface $element);
  static public function findChildByID(FieldFormElementInterface $element, $id);
  static public function findChildByName(FieldFormElementInterface $element, $name);
  static public function findChildrenByType(FieldFormElementInterface $element, $type);
  static public function settingsForm($form, &$form_state);
  static public function save(FieldFormElementInterface $element);
}

class FieldFormElementController implements FieldFormElementControllerInterface {
  protected $elementType;
  protected $elementInfo;

  public function __construct($elementType) {
    $this->elementType = $elementType;
    $this->elementInfo = fieldform_get_info($elementType);
  }

  public function create($values = array()) {
    $class = isset($this->elementInfo['element class']) && class_implements('FieldFormElement') ?
      $this->elementInfo['element class'] : 'FieldFormElement';
    return new $class($this->elementType, $values);
  }

  static public function isElementArray(array $element) {
    $types = array_keys(fieldform_get_info());
    return isset($element['type']) && in_array($element['type'], $types);
  }

  static public function sortWeights($a, $b) {
    $a_weight = (is_object($a) && isset($a->weight)) ? $a->weight : 0;
    $b_weight = (is_object($b) && isset($b->weight)) ? $b->weight : 0;
    if ($a_weight == $b_weight) {
      return 0;
    }
    return ($a_weight < $b_weight) ? -1 : 1;
  }

  static public function addElement(FieldFormElementInterface $element, $child) {
    if (is_array($child)) {
      if (!self::isElementArray($child)) {
        return FALSE;
      }
      $child = fieldform_element_create($child['type'], $child);
    }

    if (empty($child->parents)) {
      // If the parents haven't been set, add it here
      $child->parents = $element->parents;
      $child->parents[] = $child->name;
      $element->children[$child->_id] = $child;
      uasort($element->children, 'self::sortWeights');
      return $element;
    }

    $child_parents = $child->parents;
    array_pop($child_parents);
    if ($element->parents === $child_parents && !array_key_exists($child->_id, $element->children)) {
      $element->children[$child->_id] = $child;
      uasort($element->children, 'self::sortWeights');
      return $element;
    }
    elseif (count($child_parents) > $element->parents) {
      // Child is a grandchild or deeper
      foreach ($element->children as $next) {
        if (($found = $next->addElement($child)) !== FALSE) {
          return $found;
        }
      }
    }

    return FALSE;
  }

  static public function updateElement(FieldFormElementInterface $element, array $values) {
    unset($values['name'], $values['type'], $values['_id'], $values['children']); // Prevent overriding important keys
    foreach ($values as $key => $value) {
      $element->$key = $value;
    }
    return $element;
  }

  static public function deleteElement(FieldFormElementInterface $element, FieldFormElementInterface $child) {
    if ($element->hasChildren() && array_key_exists($child->_id, $element->children)) {
      unset($element->children[$child->_id]);
      return TRUE;
    }
    elseif ($element->hasChildren()) {
      foreach ($element->children as $_id => &$kid) {
        if (is_string($kid)) {
          $kid = unserialize($kid);
        }
        if ($kid->deleteElement($child) === TRUE) {
          return TRUE;
        }
      }
    }

    return FALSE;
  }

  static public function buildElement(FieldFormElementInterface $element, FieldFormEntity $entity,
                                      $form = array(), &$form_state = array()) {
    $array = $element->toArray();
    unset($array['id']);
    $field = array();

    foreach ($array as $key => $value) {
      if ($element->findChildByName($key) === FALSE) {
        $field['#'.$key] = $value;
      }
    }

    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $field[$child->name] = $child->buildElement($entity, $form, $form_state);
      }
    }

    return $field;
  }

  static public function toArray(FieldFormElementInterface $element) {
    $array = get_object_vars($element);
    unset($array['_id'], $array['parents'], $array['children']);
    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $array[$child->name] = $child->toArray();
      }
    }
    return $array;
  }

  static public function hasChild(FieldFormElementInterface $element, $id) {
    return !empty($element->children[$id]);
  }

  static public function hasChildren(FieldFormElementInterface $element) {
    return !empty($element->children);
  }

  static public function findChildByID(FieldFormElementInterface $element, $id) {
    if ($element->hasChild($id)) {
      return $element->children[$id];
    }

    foreach ($element->children as $child) {
      if (is_string($child)) {
        $child = unserialize($child);
      }
      if (($found = $child->findChildById($id)) !== FALSE) {
        return $found;
      }
    }
    return FALSE;
  }

  static public function findChildByName(FieldFormElementInterface $element, $name) {
    foreach ($element->children as $child) {
      if (is_string($child) == 1) {
        $child = unserialize($child);
      }
      if ($child->name === $name) {
        return $child;
      }
      if (($found = $child->findChildByName($name)) !== FALSE) {
        return $found;
      }
    }
    return FALSE;
  }

  static public function findChildrenByType(FieldFormElementInterface $element, $type) {
    $children = array();
    foreach ($element->children as $child) {
      if (is_string($child)) {
        $child = unserialize($child);
      }
      if ($child->elementType() === $type) {
        $children[$child->_id] = $child;
      }
      $found = $child->findChildrenByType($type);
      $children += $found;
    }
    return $children;
  }

  static public function settingsForm($form, &$form_state, FieldFormElementInterface $element = NULL, $op = NULL) {
    $i = 0;
    $values = isset($form_state['input']) && !empty($form_state['input']) ?
      $form_state['input'] : get_object_vars($element);
    $properties = $element->elementInfo()['properties'];

    $form['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#required' => TRUE,
      '#default_value' => isset($values['title']) ? $values['title'] : '',
      '#weight' => $i
    );

    $form['name'] = array(
      '#type' => 'hidden',
      '#value' => isset($values['name']) ? $values['name'] : ''
    );

    $form['type'] = array(
      '#type' => 'hidden',
      '#value' => isset($values['type']) ? $values['type'] : ''
    );

    $form['required'] = array(
      '#type' => 'checkbox',
      '#title' => t('Required'),
      '#default_value' => isset($values['required']) ? (int)($values['required'] == 1) : FALSE,
      '#weight' => $i++
    );

    $form['description'] = array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#default_value' => isset($values['description']) ? $values['description'] : '',
      '#weight' => $i++
    );

    foreach ($properties as $name => $property) {
      if ($property['type'] !== FIELDFORM_PROPERTY_NONE) {
        $value = NULL;
        if (isset($values[$name])) {
          $value = $values[$name];
        }
        elseif (isset($property['default_value'])) {
          $value = $property['default_value'];
        }
        $form[$name] = array();
        switch ($property['type']) {
          case FIELDFORM_PROPERTY_TEXT:
            $form[$name]['#type'] = 'textarea';
            break;
          case FIELDFORM_PROPERTY_MARKUP:
            $form[$name]['#type'] = 'text_format';
            break;
          case FIELDFORM_PROPERTY_SELECT:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'select';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_RADIOS:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'radios';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_CHECKBOXES:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'checkboxes';
            $form[$name]['#options'] = isset($value) ? $value : $form['options']['#options'];
            break;
          case FIELDFORM_PROPERTY_TABLE:
            $value = empty($value) ? array() : $value;
            $form[$name]['#type'] = 'namevalue_table';
            $form[$name]['#options'] = $value;
            break;
          case FIELDFORM_PROPERTY_DATE:
            $form[$name]['#type'] = 'date';
            $form[$name]['#default_value'] = isset($value) ?
              $value : array('month' => date('n'), 'day' => array('j'), 'year' => date('Y'));
            break;
          case FIELDFORM_PROPERTY_BOOL:
            $form[$name]['#type'] = 'checkbox';
            $form[$name]['#default_value'] = ($value === TRUE);
            break;
        }

        $form[$name] += array(
          '#type' => 'textfield',
          '#title' => isset($property['title']) ? $property['title'] : t(ucwords(str_replace('_', ' ', $name))),
          '#default_value' => $value,
          '#weight' => $i++
        );
      }
    }

    $form['details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Element Details'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => $i
    );

    $class = '';

    if (isset($values['attributes']) && isset($values['attributes']['class'])) {
      $class = $values['attributes']['class'];
      unset($values['attributes']['class']);
    }

    $form['details']['class'] = array(
      '#type' => 'textfield',
      '#title' => t('Class'),
      '#default_value' => $class
    );

    $form['details']['attributes'] = array(
      '#type' => 'namevalue_table',
      '#title' => t('Attributes'),
      '#options' => isset($values['attributes']) ? $values['attributes'] : array()
    );

    $form['details']['title_display'] = array(
      '#type' => 'select',
      '#title' => t('Title Display'),
      '#options' => array(
        'before' => t('Before'),
        'after' => t('Inline'),
        'invisible' => t('Hidden')
      ),
      '#default_value' => isset($values['title_display']) ? $values['title_display'] : 'before'
    );

    $form['details']['weight'] = array(
      '#type' => 'weight',
      '#title' => t('Weight'),
      '#delta' => 100,
      '#default_value' => isset($values['weight']) ? $values['weight'] : 0
    );

    return $form;
  }

  static public function settingsValidateForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
    // Just a placeholder function to extend
  }

  static public function settingsSubmitForm(&$form, &$form_state, $entity_type, $entity, $element_type, $element) {
   if (isset($form_state['input']['attributes']['values'])) {
     $form_state['input']['attributes'] = $form_state['input']['attributes']['values'];
   }

    if (isset($form_state['input']['attributes'])) {
      $form_state['input']['attributes'] = $form_state['input']['attributes'] +
        array('class' => $form_state['input']['class']);
    }
  }

  static public function save(FieldFormElementInterface $element) {
    return serialize($element);
  }

  static public function validate(FieldFormElementInterface $element, &$form, &$form_state) {
    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $child->validate($form, $form_state);
      }
    }
  }

  static public function submit(FieldFormElementInterface $element, $form, &$form_state) {
    if ($element->hasChildren()) {
      foreach ($element->children as $child) {
        $child->submit($form, $form_state);
      }
    }
  }
}