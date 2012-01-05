<?php

// Sample code for project-specific preferences.
// These prefs may include:
//
// - preferences for your application graphics
// - application selection
//  (i.e. if you have multiple apps, let user choose among them)
//
// Edit this file accordingly,
// and put your version in html/project_specific/project_specific_prefs.inc

// Select standard prefs here:

define ('COLOR_PREFS', false);
define ('GFX_CPU_PREFS', true);
define ('APP_SELECT_PREFS', true);
define ('GPU_TARGET_FREQUENCY_PREFERENCE', true);
define ('GPU_NONRESPONSIVE_PREFERENCE', false);
define ('GPU_PRIORITY_PREFERENCE', false);
define ('GPU_DISABLE_CHECKPOINTING_PREFERENCE', false);
$project_has_beta = false;

// Project-specific prefs are represented in three ways:
// - as an XML document (stored in the DB in this form)
// - as a PHP structure
// - as a set of HTTP GET form variables

// This file exports the following functions
// (called from html/inc/prefs.inc):
//
// project_specific_prefs_default()
//      Returns XML for default preferences
// project_specific_prefs_parse($xml)
//      Parse prefs as XML, return prefs as PHP structure
// project_specific_prefs_show($prefs, $columns=false)
//      Show prefs as HTML (non-editable)
// project_specific_prefs_edit($prefs, $error)
//      Show prefs as HTML, editable.
//      $error is a struct indicating which values were erroneous
//      (value X is erroneous if $error->X is set)
// project_specific_prefs_parse_form(&$error)
//      Parse form variables into XML, and return it.
//      Also error-check values, and return errors in $errors->*

function option($name, $val) {
    $x = ($name == $val) ?  "selected" : "";
    return "<option name='$name' $x>$name\n";
}

define('COLOR_DESC', tra('Color scheme for graphics'));
define("MAX_GFX_CPU_PCT_DESC", tra("Maximum CPU % for graphics%10 ... 100%2", "<br><span class=note>", "</span>"));
define('APP_SELECT_DESC', tra('Run only the selected applications'));
define('ACCEPT_ANY_DESC', tra('If no work for selected applications is available, accept work from other applications?'));

define('GPU_TARGET_FREQUENCY_DESC', tra('Frequency (in Hz) that should try to complete individual work chunks. Higher numbers may run slower but will provide a more responsive system. Lower may be faster but more laggy. (default 30.0)'));
define('GPU_NONRESPONSIVE_DESC', tra('WARNING: Be careful with this option. Ignore any responsiveness requirements for GPU workunits. This could result in an unusable system while it is running.'));
define('GPU_PRIORITY_DESC', tra('Optionally change the process priority that a GPU application will use'));
define('GPU_DISABLE_CHECKPOINTING_DESC', tra('Disable main checkpointing for GPU applications.'));

// stuff related to app filtering.
// Note: in this implementation, if a user selects all apps,
// no <app_id> elements are included in their prefs,
// which means that if the project adds a new app such users will run it also.
//
if (APP_SELECT_PREFS) {
    $app_array = array();
    $apps = BoincApp::enum("deprecated=0");
    foreach($apps as $app) {
        $app_array[] = array($app->id, $app->user_friendly_name);
    }
} else {
    $app_array = null;
}

function selected_app_text($prefs) {
    global $app_array;
    if (isset($prefs->app_ids)) {
        $x = "";
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $app_name = $app[1];
            if (in_array($app_id, $prefs->app_ids)) {
                $x .= "$app_name: ".tra("yes")."<br>";
            } else {
                $x .= "$app_name: ".tra("no")."<br>";
            }
        }
    } else {
        $x = tra("(all applications)");
    }
    return $x;
}

function project_specific_prefs_default() {
    $x = "";
    if (COLOR_PREFS) {
        $x .= "<color_scheme>Tahiti Sunset</color_scheme>\n";
    }
    if (GFX_CPU_PREFS) {
        $x .= "<max_gfx_cpu_pct>20</max_gfx_cpu_pct>\n";
    }

    if (GPU_TARGET_FREQUENCY_PREFERENCE) {
        $x .= "<gpu_target_frequency>30.0</gpu_target_frequency>\n";
    }

    if (GPU_NONRESPONSIVE_PREFERENCE) {
        $x .= "<gpu_non_responsive>0</gpu_non_responsive>\n";
    }

    if (GPU_DISABLE_CHECKPOINTING_PREFERENCE) {
        $x .= "<no_gpu_checkpoint>0</no_gpu_checkpoint>\n";
    }

    if (GPU_PRIORITY_PREFERENCE) {
        $x .= "<gpu_process_priority>2</gpu_process_priority>\n";
    }

    return $x;
}

function project_specific_prefs_edit($prefs, $error=false) {
    global $app_array;
    if (COLOR_PREFS) {
        $x = $prefs->color_scheme;
        $y = "<select name=color_scheme>
                ".option("Tahiti Sunset", $x)
                .option("Desert Sands", $x)."
            </select>
        ";
        row2(COLOR_DESC, $y);
    }

    if (GFX_CPU_PREFS) {
        $y = "<input size=5 name=max_gfx_cpu_pct value='$prefs->max_gfx_cpu_pct'>";
        row2(MAX_GFX_CPU_PCT_DESC, $y, isset($error->max_gfx_cpu_pct));
    }
    if (APP_SELECT_PREFS) {
        $x = "";
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $app_name = $app[1];
            if (isset($prefs->app_ids)) {
                $present = in_array($app_id, $prefs->app_ids);
            } else {
                $present = true;
            }
            $checked = $present?"checked":"";
            $x .= "<input type=checkbox name=app_id_$app_id $checked> $app_name<br>";
        }
        row2(APP_SELECT_DESC, $x);
        $checked = $prefs->allow_non_preferred_apps?"checked":"";
        row2(ACCEPT_ANY_DESC, "<input type=checkbox name=allow_non_preferred_apps $checked>");
    }

    if (GPU_TARGET_FREQUENCY_PREFERENCE) {
        $y = "<input size=5 name=gpu_target_frequency value='$prefs->gpu_target_frequency'>";
        row2(GPU_TARGET_FREQUENCY_DESC, $y, isset($error->gpu_target_frequency));
    }

}

function project_specific_prefs_parse_form(&$error) {
    global $app_array;
    $x = "";
    if (COLOR_PREFS) {
        $color_scheme = sanitize_tags($_GET["color_scheme"]);
        $x .= "<color_scheme>$color_scheme</color_scheme>\n";
    }
    if (GFX_CPU_PREFS) {
        $max_gfx_cpu_pct = sanitize_numeric($_GET["max_gfx_cpu_pct"]);
        if (!verify_numeric($max_gfx_cpu_pct, 0, 100)) $error->max_gfx_cpu_pct = true;
        $x .= "<max_gfx_cpu_pct>$max_gfx_cpu_pct</max_gfx_cpu_pct>\n";
    }

    if (APP_SELECT_PREFS) {
        $y = "<apps_selected>\n";
        $some_unchecked = false;
        foreach ($app_array as $app) {
            $app_id = $app[0];
            $present = isset($_GET["app_id_$app_id"]);
            if ($present) {
                $y .= "<app_id>$app_id</app_id>\n";
            } else {
                $some_unchecked = true;
            }
        }
        $y .= "</apps_selected>\n";

        if ($some_unchecked) {
            $x .= $y;
        }
        if (isset($_GET["allow_non_preferred_apps"])) {
            $x .= "<allow_non_preferred_apps>1</allow_non_preferred_apps>\n";
        }
    }

    if (GPU_TARGET_FREQUENCY_PREFS) {
        $gpu_target_frequency = sanitize_numeric($_GET["gpu_target_frequency"]);
        if (!verify_numeric($gpu_target_frequency, 0, 100000000)) $error->gpu_target_frequency = true;
        $x .= "<gpu_target_frequency>$gpu_target_frequency</gpu_target_frequency>\n";
    }

    return $x;
}

function project_specific_prefs_show($prefs, $columns=false) {
    global $app_array;
    if ($columns) {
        if (COLOR_PREFS) {
            row_defs(COLOR_DESC,"color_scheme", "", "", $prefs);
        }
        if (GFX_CPU_PREFS) {
            row_defs(MAX_GFX_CPU_PCT_DESC, "max_gfx_cpu_pct", "", "", $prefs);
        }
        if (APP_SELECT_PREFS) {
            $prefs->app_id_text = selected_app_text($prefs);
            if ($prefs->home) $prefs->home->app_id_text = selected_app_text($prefs->home);
            if ($prefs->school) $prefs->school->app_id_text = selected_app_text($prefs->school);
            if ($prefs->work) $prefs->work->app_id_text = selected_app_text($prefs->work);
            row_defs(APP_SELECT_DESC, "app_id_text", "", "", $prefs);
            row_defs(ACCEPT_ANY_DESC, "allow_non_preferred_apps_text", "", "", $prefs);
        }

        if (GPU_TARGET_FREQUENCY_PREFERENCE) {
            row_defs(GPU_TARGET_FREQUENCY_DESC, "gpu_target_frequency", "", "", $prefs);
        }

        if (GPU_DISABLE_CHECKPOINTING_PREFERENCE) {

        }

        if (GPU_NONRESPONSIVE_PREFERENCE) {

        }

        if (GPU_PRIORITY_PREFERENCE) {

        }
    } else {
        if (COLOR_PREFS) {
            row2(COLOR_DESC, $prefs->color_scheme);
        }
        if (GFX_CPU_PREFS) {
            row2(MAX_GFX_CPU_PCT_DESC, $prefs->max_gfx_cpu_pct);
        }
        if (APP_SELECT_PREFS) {
            row2(APP_SELECT_DESC, selected_app_text($prefs));
            row2(ACCEPT_ANY_DESC, $prefs->allow_non_preferred_apps_text);
        }

        if (GPU_TARGET_FREQUENCY_PREFERENCE) {
            row2(GPU_TARGET_FREQUENCY_DESC, $prefs->gpu_target_frequency);
        }

        if (GPU_DISABLE_CHECKPOINTING_PREFERENCE) {

        }

        if (GPU_NONRESPONSIVE_PREFERENCE) {

        }

        if (GPU_PRIORITY_PREFERENCE) {

        }
    }
}

function project_specific_prefs_parse($prefs_xml) {
    if (COLOR_PREFS) {
        $prefs->color_scheme = parse_element($prefs_xml, "<color_scheme>");
    }
    if (GFX_CPU_PREFS) {
        $prefs->max_gfx_cpu_pct = parse_element($prefs_xml, "<max_gfx_cpu_pct>");
    }
    if (APP_SELECT_PREFS) {
        $cursor = 0;
        while ($thisxml = parse_next_element($prefs_xml, "<app_id>", $cursor)) {
            if (is_numeric($thisxml)) {
                $n = (int) $thisxml;
                $prefs->app_ids[] = $n;
            }
        }
        $prefs->allow_non_preferred_apps = parse_element($prefs_xml, "<allow_non_preferred_apps>");
        $prefs->allow_non_preferred_apps_text = $prefs->allow_non_preferred_apps?"yes":"no";
    }

    if (GPU_TARGET_FREQUENCY_PREFERENCE) {
        $prefs->gpu_target_frequency = parse_element($prefs_xml, "<gpu_target_frequency>");
    }

    if (GPU_DISABLE_CHECKPOINTING_PREFERENCE) {
        $prefs->disable_gpu_checkpointing = parse_element($prefs_xml, "<no_gpu_checkpoint>");
        $prefs->disable_gpu_checkpointing_text = $prefs->disable_gpu_checkpointing?"yes":"no";
    }

    if (GPU_NONRESPONSIVE_PREFERENCE) {

    }

    if (GPU_PRIORITY_PREFERENCE) {

    }


    return $prefs;
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
