<?php

/**
 * @file
 * Preprocess.
 */

use Drupal\Component\Utility\Html;
use Drupal\Core\Template\Attribute;

/**
 * Implements hook_preprocess_HOOK() for HTML document templates.
 */
function frost_theme_preprocess_html(&$vars) {
  $request = \Drupal::request();
  $route_match = \Drupal::routeMatch();
  $title = \Drupal::service('title_resolver')
    ->getTitle($request, $route_match->getRouteObject());

  // Classes to be applied to the body, but overridden if fields present.
  $field_hero_class = Html::cleanCssIdentifier('page-lacks--field-hero');
  $field_sections_class = Html::cleanCssIdentifier('page-lacks--field-sections');

  // Get classes out of the twig.
  if (!isset($vars['body_classes'])) {
    $vars['body_classes'] = [];
  }

  // Logged-in body class.
  if (isset($vars['logged_in']) && $vars['logged_in']) {
    $vars['body_classes'][] = 'user-logged-in';
  }

  // Offline body class.
  if (isset($vars['db_offline']) && $vars['db_offline']) {
    $vars['body_classes'][] = 'db-offline';
  }

  // Root path body class.
  $vars['body_classes'][] = (isset($vars['root_path']) && !$vars['root_path']) ? 'path-frontpage' : Html::cleanCssIdentifier('path-' . $vars['root_path']);

  // Node type body class.
  if (isset($vars['node_type']) && $vars['node_type']) {
    $vars['body_classes'][] = Html::cleanCssIdentifier('node--type-' . $vars['node_type']);
  }

  // Perform operations based on page context (node, term, etc.).
  $parameters = \Drupal::routeMatch()->getParameters()->all();
  $paged_entity_types = ['node', 'taxonomy_term', 'commerce_product'];
  foreach ($paged_entity_types as $type) {
    if (isset($parameters[$type])) {
      $entity = $parameters[$type];
      $bundle = $entity->bundle();

      // D7-style body classes.
      $vars['body_classes'][] = Html::cleanCssIdentifier($type . '--' . $entity->id());
      $vars['body_classes'][] = Html::cleanCssIdentifier($type . '-' . $bundle . '--' . $entity->id());
      $vars['body_classes'][] = Html::cleanCssIdentifier($type . '--' . $bundle);
      $vars['body_classes'][] = Html::cleanCssIdentifier('page--' . $type);
      $vars['body_classes'][] = Html::cleanCssIdentifier('page--' . $type . '-' . $bundle);

      // Body class for the presence of field_hero.
      $view_displays = \Drupal::service('entity_display.repository')
        ->getViewModeOptionsByBundle($type, $bundle);
      if ($entity->hasField('field_hero')) {
        $field_hero_class = Html::cleanCssIdentifier('page-has--field-hero');
      }
      elseif (array_key_exists('hero', $view_displays)) {
        $field_hero_class = Html::cleanCssIdentifier('page-has--hero-view-mode');
      }

      // Body class for the presence of field_sections.
      if ($entity->hasField('field_sections')) {
        $field_sections_class = Html::cleanCssIdentifier('page-has--field-sections');
      }
    }
  }

  // Attach classes now that the page has been preprocessed for them.
  $vars['body_classes'][] = $field_hero_class;
  $vars['body_classes'][] = $field_sections_class;

  // Meta robots noindex for paginated content.
  if ($query = \Drupal::request()->query->get('page')) {
    $robotted = FALSE;
    foreach ($vars['page']['#attached']['html_head'] as $key => $tag) {
      if (isset($tag[1]) && $tag[1] === 'robots') {
        $vars['page']['#attached']['html_head'][$key][0]['#attributes']['content'] = 'noindex, follow';

        $robotted = TRUE;
        break;
      }
    }

    // In case the loop didn't find anything.
    if (!$robotted) {
      $vars['page']['#attached']['html_head'][] = [
        [
          '#tag' => 'meta',
          '#attributes' => [
            'name' => 'robots',
            'content' => 'noindex, follow',
          ],
        ],
        'robots',
      ];
      $robotted = TRUE;
    }
  }

  // Use standard title pattern when SEO field not populated.
  $site = Drupal::config('system.site');
  $site_name = $site->get('name');
  if (isset($vars['head_title']['title'])
    && $vars['head_title']['title'] === '| ' . $site_name
  ) {
    $vars['head_title']['title'] = $title . ' | ' . $site_name;
  }
}
