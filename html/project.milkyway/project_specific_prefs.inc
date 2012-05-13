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


// GPU usage preferences
define ('GPU_TARGET_FREQUENCY_PREF', true);
define ('GPU_NONRESPONSIVE_PREF', false);
define ('GPU_PRIORITY_PREF', false);
define ('GPU_DISABLE_CHECKPOINTING_PREF', false);

define ('GPU_TARGET_FREQUENCY_DEFAULT', 60.0);

// nbody screensaver preferences
define ('NBODY_GRAPHICS_POLL_PERIOD_PREF', true);
define ('NBODY_GRAPHICS_FPS_PREF', false);
define ('NBODY_GRAPHICS_BLOCK_SIMULATION_PREF',true);
define ('NBODY_GRAPHICS_FLOAT_PREF', true);
define ('NBODY_GRAPHICS_FLOAT_SPEED_PREF', true);
define ('NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF', true);
define ('NBODY_GRAPHICS_POINT_POINT_SIZE_PREF', true);
define ('NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF', true);
define ('NBODY_GRAPHICS_MONOCHROMATIC_PREF', true);
define ('NBODY_GRAPHICS_ORIGIN_CENTERED_PREF', true);
define ('NBODY_GRAPHICS_SHOW_INFO_PREF', true);
define ('NBODY_GRAPHICS_SHOW_AXES_PREF', true);
define ('NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF', true);

define ('NBODY_GRAPHICS_POLL_PERIOD_DEFAULT', 30);
define ('NBODY_GRAPHICS_FPS_DEFAULT', 30);
define ('NBODY_GRAPHICS_BLOCK_SIMULATION_DEFAULT', 0); // false
define ('NBODY_GRAPHICS_FLOAT_DEFAULT', 1); // true
define ('NBODY_GRAPHICS_FLOAT_SPEED_DEFAULT', 5.0);
define ('NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DEFAULT', 250.0);
define ('NBODY_GRAPHICS_POINT_POINT_SIZE_DEFAULT', 40.0);
define ('NBODY_GRAPHICS_UNTEXTURED_POINTS_DEFAULT', 0); // false
define ('NBODY_GRAPHICS_MONOCHROMATIC_DEFAULT', 0); // false
define ('NBODY_GRAPHICS_ORIGIN_CENTERED_DEFAULT', 0); // false
define ('NBODY_GRAPHICS_SHOW_INFO_DEFAULT', 1); // true
define ('NBODY_GRAPHICS_SHOW_AXES_DEFAULT', 0); // false
define ('NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DEFAULT', 0); // false

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


// standard
define('COLOR_DESC', tra('Color scheme for graphics'));
define("MAX_GFX_CPU_PCT_DESC", tra("Maximum CPU % for graphics%10 ... 100%2", "<br><span class=note>", "</span>"));
define('APP_SELECT_DESC', tra('Run only the selected applications'));
define('ACCEPT_ANY_DESC', tra('If no work for selected applications is available, accept work from other applications?'));

// GPU preference descriptions
define('GPU_TARGET_FREQUENCY_DESC', tra('Frequency (in Hz) that should try to complete individual work chunks. Higher numbers may run slower but will provide a more responsive system. Lower may be faster but more laggy. %1default 60 (corresponds to 60 fps)%2', "<br><span class=note>", "</span>"));
define('GPU_NONRESPONSIVE_DESC', tra('WARNING: Be careful with this option. Ignore any responsiveness requirements for GPU workunits. This could result in an unusable system while it is running.'));
define('GPU_PRIORITY_DESC', tra('Optionally change the process priority that a GPU application will use'));
define('GPU_DISABLE_CHECKPOINTING_DESC', tra('Disable main checkpointing for GPU applications.'));

// nbody screensaver preferences descriptions

define('NBODY_GRAPHICS_POLL_PERIOD_DESC', tra('Period (in milliseconds) to check for scene updates'));
define('NBODY_GRAPHICS_FPS_DESC', tra('Reserved (use poll period)'));
define('NBODY_GRAPHICS_BLOCK_SIMULATION_DESC', tra('Simulation should wait for graphics to display every timestep as a frame'));

define('NBODY_GRAPHICS_FLOAT_DESC', tra('View randomly floats around by default'));
define('NBODY_GRAPHICS_FLOAT_SPEED_DESC', tra('Change how fast the view randomly moves around'));
define('NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DESC', tra('Make particles bigger or smaller'));
define('NBODY_GRAPHICS_POINT_POINT_SIZE_DESC', tra('Make particles bigger or smaller when using uglier particles'));

define('NBODY_GRAPHICS_UNTEXTURED_POINTS_DESC', tra('Use uglier particles that may be somewhat faster'));
define('NBODY_GRAPHICS_MONOCHROMATIC_DESC', tra('Make all particles the same color'));
define('NBODY_GRAPHICS_ORIGIN_CENTERED_DESC', tra('View should focus on the origin rather than the center of mass of the simulation'));

define('NBODY_GRAPHICS_SHOW_INFO_DESC', tra('Show text description and process information from corner'));
define('NBODY_GRAPHICS_SHOW_AXES_DESC', tra('Show reference XYZ axes at center'));
define('NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DESC', tra('Show path the center of mass has taken'));

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

    if (GPU_TARGET_FREQUENCY_PREF) {
        $x .= "<gpu_target_frequency>" . GPU_TARGET_FREQUENCY_DEFAULT . "</gpu_target_frequency>\n";
    }

    if (GPU_NONRESPONSIVE_PREF) {
        $x .= "<gpu_non_responsive>0</gpu_non_responsive>\n";
    }

    if (GPU_DISABLE_CHECKPOINTING_PREF) {
        $x .= "<no_gpu_checkpoint>0</no_gpu_checkpoint>\n";
    }

    if (GPU_PRIORITY_PREF) {
        $x .= "<gpu_process_priority>2</gpu_process_priority>\n";
    }


    if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
        $x .= "<nbody_graphics_poll_period>" . NBODY_GRAPHICS_POLL_PERIOD_DEFAULT . "</nbody_graphics_poll_period>\n";
    }

    if (NBODY_GRAPHICS_FPS_PREF) {
        $x .= "<nbody_graphics_fps>" . NBODY_GRAPHICS_FPS_DEFAULT . "</nbody_graphics_fps>\n";
    }

    if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
        $x .= "<nbody_graphics_block_simulation>" . NBODY_GRAPHICS_BLOCK_SIMULATION_DEFAULT . "</nbody_graphics_block_simulation>\n";
    }

    if (NBODY_GRAPHICS_FLOAT_PREF) {
        $x .= "<nbody_graphics_float>" . NBODY_GRAPHICS_FLOAT_DEFAULT . "</nbody_graphics_float>\n";
    }

    if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
        $x .= "<nbody_graphics_float_speed>" . NBODY_GRAPHICS_FLOAT_SPEED_DEFAULT . "</nbody_graphics_float_speed>\n";
    }

    if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
        $x .= "<nbody_graphics_textured_point_size>" . NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DEFAULT . "</nbody_graphics_textured_point_size>\n";
    }

    if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
        " . x .= " . "<nbody_graphics_point_point_size>" . NBODY_GRAPHICS_POINT_POINT_SIZE_DEFAULT . "</nbody_graphics_point_point_size>\n";
    }

    if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
        $x .= "<nbody_graphics_untextured_points>" . NBODY_GRAPHICS_UNTEXTURED_POINTS_DEFAULT . "</nbody_graphics_untextured_points>\n";
    }

    if (NBODY_GRAPHICS_MONOCHROMATIC_PREF) {
        $x .= "<nbody_graphics_monochromatic>" . NBODY_GRAPHICS_MONOCHROMATIC_DEFAULT . "</nbody_graphics_monochromatic>\n";
    }

    if (NBODY_GRAPHICS_ORIGIN_CENTERED_PREF) {
        $x .= "<nbody_graphics_origin_centered>" . NBODY_GRAPHICS_ORIGIN_CENTERED_DEFAULT . "</nbody_graphics_origin_centered>\n";
    }

    if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
        $x .= "<nbody_graphics_show_info>" . NBODY_GRAPHICS_SHOW_INFO_DEFAULT . "</nbody_graphics_show_info>\n";
    }

    if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
        $x .= "<nbody_graphics_show_axes>" . NBODY_GRAPHICS_SHOW_AXES_DEFAULT . "</nbody_graphics_show_axes>\n";
    }

    if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
        $x .= "<nbody_graphics_show_orbit_trace>" . NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DEFAULT . "</nbody_graphics_show_orbit_trace>\n";
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

    if (GPU_TARGET_FREQUENCY_PREF) {
        $y = "<input size=5 name=gpu_target_frequency value='$prefs->gpu_target_frequency'>";
        row2(GPU_TARGET_FREQUENCY_DESC, $y, isset($error->gpu_target_frequency));
    }

    if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
        $y = "<input size=5 name=nbody_graphics_poll_period value='$prefs->nbody_graphics_poll_period'>";
        row2(NBODY_GRAPHICS_POLL_PERIOD_DESC, $y, isset($error->nbody_graphics_poll_period));
    }

    if (NBODY_GRAPHICS_FPS_PREF) {
        $y = "<input size=5 name=nbody_graphics_fps value='$prefs->nbody_graphics_fps'>";
        row2(NBODY_GRAPHICS_FPS_DESC, $y, isset($error->nbody_graphics_fps));
    }

    if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
        $checked = $prefs->nbody_graphics_block_simulation ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_block_simulation $checked>";
        row2(NBODY_GRAPHICS_BLOCK_SIMULATION_DESC, $y, isset($error->nbody_graphics_block_simulation));
    }

    if (NBODY_GRAPHICS_FLOAT_PREF) {
        $checked = $prefs->nbody_graphics_float ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_float $checked>";
        row2(NBODY_GRAPHICS_FLOAT_DESC, $y, isset($error->nbody_graphics_float));
    }

    if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
        if (!isset($prefs->nbody_graphics_float_speed)) {
            $prefs->nbody_graphics_float_speed = NBODY_GRAPHICS_FLOAT_SPEED_DEFAULT;
        }
        $y = "<input size=5 name=nbody_graphics_float_speed value='$prefs->nbody_graphics_float_speed'>";
        row2(NBODY_GRAPHICS_FLOAT_SPEED_DESC, $y, isset($error->nbody_graphics_float_speed));
    }

    if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
        $y = "<input size=5 name=nbody_graphics_texture_point_size value='$prefs->nbody_graphics_textured_point_size'>";
        row2(NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DESC, $y, isset($error->nbody_graphics_textured_point_size));
    }

    if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
        $y = "<input size=5 name=nbody_graphics_point_point_size value='$prefs->nbody_graphics_point_point_size'>";
        row2(NBODY_GRAPHICS_POINT_POINT_SIZE_DESC, $y, isset($error->nbody_graphics_point_point_size));
    }

    if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
        $y = "<input type=checkbox name=nbody_graphics_untextured_points value='$prefs->nbody_graphics_untextured_points'>";
        row2(NBODY_GRAPHICS_UNTEXTURED_POINTS_DESC, $y, isset($error->nbody_graphics_untextured_points));
    }

    if (NBODY_GRAPHICS_MONOCHROMATIC_PREF) {
        $checked = $prefs->nbody_graphics_monochromatic ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_monochromatic $checked>";
        row2(NBODY_GRAPHICS_MONOCHROMATIC_DESC, $y, isset($error->nbody_graphics_monochromatic));
    }

    if (NBODY_GRAPHICS_ORIGIN_CENTERED_PREF) {
        $checked = $prefs->nbody_graphics_origin_centered ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_origin_centered $checked>";
        row2(NBODY_GRAPHICS_ORIGIN_CENTERED_DESC, $y, isset($error->nbody_graphics_origin_centered));
    }

    if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
        $checked = $prefs->nbody_graphics_show_info ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_show_info $checked>";
        row2(NBODY_GRAPHICS_SHOW_INFO_DESC, $y, isset($error->nbody_graphics_show_info));
    }

    if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
        $y = "<input type=checkbox name=nbody_graphics_show_axes value='$prefs->nbody_graphics_show_axes'>";
        row2(NBODY_GRAPHICS_SHOW_AXES_DESC, $y, isset($error->nbody_graphics_show_axes));
    }

    if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
        $checked = $prefs->nbody_graphics_show_orbit_trace ? "checked" : "";
        $y = "<input type=checkbox name=nbody_graphics_show_orbit_trace $checked>";
        row2(NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DESC, $y, isset($error->nbody_graphics_show_orbit_trace));
    }
}

// if variable is null use given as default
function sanitize_numeric_with_default($x, $val) {
    if (is_numeric($x)) {
        return $x;
    } else if ($x == "") {
        return $val;
    } else {
        return "not numeric";
    }
}

function isset_with_default($x, $val) {

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

    if (GPU_TARGET_FREQUENCY_PREF) {
        $gpu_target_frequency = sanitize_numeric_with_default($_GET["gpu_target_frequency"], GPU_TARGET_FREQUENCY_DEFAULT);
        if (!verify_numeric($gpu_target_frequency, 0, 100000000)) $error->gpu_target_frequency = true;
        $x .= "<gpu_target_frequency>$gpu_target_frequency</gpu_target_frequency>\n";
    }

    if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
        $nbody_graphics_poll_period = sanitize_numeric_with_default($_GET["nbody_graphics_poll_period"], NBODY_GRAPHICS_POLL_PERIOD_DEFAULT);
        if (!verify_numeric($nbody_graphics_poll_period, 0, 5000)) $error->nbody_graphics_poll_period = true;
        $x .= "<nbody_graphics_poll_period>$nbody_graphics_poll_period</nbody_graphics_poll_period>\n";
    }

    if (NBODY_GRAPHICS_FPS_PREF) {
        $nbody_graphics_fps = sanitize_numeric_with_default($_GET["nbody_graphics_fps"], NBODY_GRAPHICS_FPS_DEFAULT);
        if (!verify_numeric($nbody_graphics_fps, 0, 200)) $error->nbody_graphics_fps = true;
        $x .= "<nbody_graphics_fps>$nbody_graphics_fps</nbody_graphics_fps>\n";
    }


    if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
        $nbody_graphics_block_simulation = isset($_GET["nbody_graphics_block_simulation"]) ? 1 : 0;
        $x .= "<nbody_graphics_block_simulation>$nbody_graphics_block_simulation</nbody_graphics_block_simulation>\n";
    }

    if (NBODY_GRAPHICS_FLOAT_PREF) {
        $nbody_graphics_float = isset($_GET["nbody_graphics_float"]);
        $x .= "<nbody_graphics_float>$nbody_graphics_float</nbody_graphics_float>\n";
    }

    if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
        $nbody_graphics_float_speed = sanitize_numeric_with_default($_GET["nbody_graphics_float_speed"], NBODY_GRAPHICS_FLOAT_SPEED_DEFAULT);
        if (!verify_numeric($nbody_graphics_float_speed,  1.0E-3, 30.0)) $error->nbody_graphics_float_speed = true;
        $x .= "<nbody_graphics_float_speed>$nbody_graphics_float_speed</nbody_graphics_float_speed>\n";
    }

    if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
        $nbody_graphics_textured_point_size = sanitize_numeric_with_default($_GET["nbody_graphics_textured_point_size"], NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DEFAULT);
        if (!verify_numeric($nbody_graphics_textured_point_size,  1.0E-3, 2000.0)) $error->nbody_graphics_textured_point_size = true;
        $x .= "<nbody_graphics_textured_point_size>$nbody_graphics_textured_point_size</nbody_graphics_textured_point_size>\n";
    }

    if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
        $nbody_graphics_point_point_size = sanitize_numeric_with_default($_GET["nbody_graphics_point_point_size"], NBODY_GRAPHICS_POINT_POINT_SIZE_DEFAULT);
        if (!verify_numeric($nbody_graphics_point_point_size,  1.0E-3, 500.0)) $error->nbody_graphics_point_point_size = true;
        $x .= "<nbody_graphics_point_point_size>$nbody_graphics_point_point_size</nbody_graphics_point_point_size>\n";
    }

    if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
        $nbody_graphics_untextured_points = isset($_GET["nbody_graphics_untextured_points"]) ? 1 : 0;
        $x .= "<nbody_graphics_untextured_points>$nbody_graphics_untextured_points</nbody_graphics_untextured_points>\n";
    }

    if (NBODY_GRAPHICS_MONOCHROMATIC_PREF) {
        $nbody_graphics_monochromatic = isset($_GET["nbody_graphics_monochromatic"]) ? 1 : 0;
        $x .= "<nbody_graphics_monochromatic>$nbody_graphics_monochromatic</nbody_graphics_monochromatic>\n";
    }

    if (NBODY_GRAPHICS_ORIGIN_CENTERED_PREF) {
        $nbody_graphics_origin_centered = isset($_GET["nbody_graphics_origin_centered"]) ? 1 : 0;
        $x .= "<nbody_graphics_origin_centered>$nbody_graphics_origin_centered</nbody_graphics_origin_centered>\n";
    }

    if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
        $nbody_graphics_show_info = isset($_GET["nbody_graphics_show_info"]) ? 1 : 0;
        $x .= "<nbody_graphics_show_info>$nbody_graphics_show_info</nbody_graphics_show_info>\n";
    }

    if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
        $nbody_graphics_show_axes = isset($_GET["nbody_graphics_show_axes"]) ? 1 : 0;
        $x .= "<nbody_graphics_show_axes>$nbody_graphics_show_axes</nbody_graphics_show_axes>\n";
    }

    if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
        $nbody_graphics_show_orbit_trace = isset($_GET["nbody_graphics_show_orbit_trace"]) ? 1 : 0;
        $x .= "<nbody_graphics_show_orbit_trace>$nbody_graphics_show_orbit_trace</nbody_graphics_show_orbit_trace>\n";
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

        if (GPU_TARGET_FREQUENCY_PREF) {
            row_defs(GPU_TARGET_FREQUENCY_DESC, "gpu_target_frequency", "", "", $prefs);
        }

        if (GPU_DISABLE_CHECKPOINTING_PREF) {

        }

        if (GPU_NONRESPONSIVE_PREF) {

        }

        if (GPU_PRIORITY_PREF) {

        }


        if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
            row_defs(NBODY_GRAPHICS_POLL_PERIOD_DESC, "nbody_graphics_poll_period", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_FPS_PREF) {
            row_defs(NBODY_GRAPHICS_FPS_DESC, "nbody_graphics_fps", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
            row_defs(NBODY_GRAPHICS_BLOCK_SIMULATION_DESC, "nbody_graphics_block_simulation", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_FLOAT_PREF) {
            row_defs(NBODY_GRAPHICS_FLOAT_DESC, "nbody_graphics_float", "", "", $prefs);
        }


        if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
            row_defs(NBODY_GRAPHICS_FLOAT_SPEED_DESC, "nbody_graphics_float_speed", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
            row_defs(NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DESC, "nbody_graphics_textured_point_size", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
            row_defs(NBODY_GRAPHICS_POINT_POINT_SIZE_DESC, "nbody_graphics_point_point_size", "", "", $prefs);
        }


        if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
            row_defs(NBODY_GRAPHICS_UNTEXTURED_POINTS_DESC, "nbody_graphics_untextured_points", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_MONOCHROMATIC) {
            row_defs(NBODY_GRAPHICS_MONOCHROMATIC_DESC, "nbody_graphics_monochromatic", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_ORIGIN_CENTERED) {
            row_defs(NBODY_GRAPHICS_ORIGIN_CENTERED_DESC, "nbody_graphics_origin_centered", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
            row_defs(NBODY_GRAPHICS_SHOW_INFO_DESC, "nbody_graphics_show_info", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
            row_defs(NBODY_GRAPHICS_SHOW_AXES_DESC, "nbody_graphics_show_axes", "", "", $prefs);
        }

        if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
            row_defs(NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DESC, "nbody_graphics_show_orbit_trace", "", "", $prefs);
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

        if (GPU_TARGET_FREQUENCY_PREF) {
            row2(GPU_TARGET_FREQUENCY_DESC, $prefs->gpu_target_frequency);
        }

        if (GPU_DISABLE_CHECKPOINTING_PREF) {

        }

        if (GPU_NONRESPONSIVE_PREF) {

        }

        if (GPU_PRIORITY_PREF) {

        }



        if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
            row2(NBODY_GRAPHICS_POLL_PERIOD_DESC, $prefs->nbody_graphics_poll_period);
        }

        if (NBODY_GRAPHICS_FPS_PREF) {
            row2(NBODY_GRAPHICS_FPS_DESC, $prefs->nbody_graphics_fps);
        }

        if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
            row2(NBODY_GRAPHICS_BLOCK_SIMULATION_DESC, $prefs->nbody_graphics_block_simulation);
        }

        if (NBODY_GRAPHICS_FLOAT_PREF) {
            row2(NBODY_GRAPHICS_FLOAT_DESC, $prefs->nbody_graphics_float);
        }

        if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
            row2(NBODY_GRAPHICS_FLOAT_SPEED_DESC, $prefs->nbody_graphics_float_speed);
        }

        if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
            row2(NBODY_GRAPHICS_TEXTURED_POINT_SIZE_DESC, $prefs->nbody_graphics_textured_point_size);
        }

        if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
            row2(NBODY_GRAPHICS_POINT_POINT_SIZE_DESC, $prefs->nbody_graphics_point_point_size);
        }

        if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
            row2(NBODY_GRAPHICS_UNTEXTURED_POINTS_DESC, $prefs->nbody_graphics_untextured_points);
        }

        if (NBODY_GRAPHICS_MONOCHROMATIC_PREF) {
            row2(NBODY_GRAPHICS_MONOCHROMATIC_DESC, $prefs->nbody_graphics_monochromatic);
        }

        if (NBODY_GRAPHICS_ORIGIN_CENTERED_PREF) {
            row2(NBODY_GRAPHICS_ORIGIN_CENTERED_DESC, $prefs->nbody_graphics_origin_centered);
        }

        if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
            row2(NBODY_GRAPHICS_SHOW_INFO_DESC, $prefs->nbody_graphics_show_info);
        }

        if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
            row2(NBODY_GRAPHICS_SHOW_AXES_DESC, $prefs->nbody_graphics_show_axes);
        }

        if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
            row2(NBODY_GRAPHICS_SHOW_ORBIT_TRACE_DESC, $prefs->nbody_graphics_show_orbit_trace);
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

    if (GPU_TARGET_FREQUENCY_PREF) {
        $prefs->gpu_target_frequency = parse_element($prefs_xml, "<gpu_target_frequency>");
    }

    if (GPU_DISABLE_CHECKPOINTING_PREF) {
        $prefs->disable_gpu_checkpointing = parse_element($prefs_xml, "<no_gpu_checkpoint>");
        $prefs->disable_gpu_checkpointing_text = $prefs->disable_gpu_checkpointing?"yes":"no";
    }

    if (GPU_NONRESPONSIVE_PREF) {

    }

    if (GPU_PRIORITY_PREF) {

    }


    if (NBODY_GRAPHICS_POLL_PERIOD_PREF) {
        $prefs->nbody_graphics_poll_period = parse_element($prefs_xml, "<nbody_graphics_poll_period>");
    }

    if (NBODY_GRAPHICS_FPS_PREF) {
        $prefs->nbody_graphics_fps = parse_element($prefs_xml, "<nbody_graphics_fps>");
    }

    if (NBODY_GRAPHICS_BLOCK_SIMULATION_PREF) {
        $prefs->nbody_graphics_block_simulation = parse_element($prefs_xml, "<nbody_graphics_block_simulation>");
        $prefs->nbody_graphics_block_simulation = $prefs->nbody_graphics_block_simulation?"yes":"no";
    }

    if (NBODY_GRAPHICS_FLOAT_PREF) {
        $prefs->nbody_graphics_float = parse_element($prefs_xml, "<nbody_graphics_float>");
        $prefs->nbody_graphics_float = $prefs->nbody_graphics_float?"yes":"no";
    }

    if (NBODY_GRAPHICS_FLOAT_SPEED_PREF) {
        $prefs->nbody_graphics_float_speed = parse_element($prefs_xml, "<nbody_graphics_float_speed>");
    }

    if (NBODY_GRAPHICS_TEXTURED_POINT_SIZE_PREF) {
        $prefs->nbody_graphics_textured_point_size = parse_element($prefs_xml, "<nbody_graphics_textured_point_size>");
    }

    if (NBODY_GRAPHICS_POINT_POINT_SIZE_PREF) {
        $prefs->nbody_graphics_point_point_size = parse_element($prefs_xml, "<nbody_graphics_point_point_size>");
    }

    if (NBODY_GRAPHICS_UNTEXTURED_POINTS_PREF) {
        $prefs->nbody_graphics_untextured_points = parse_element($prefs_xml, "<nbody_graphics_untextured_points>");
        $prefs->nbody_graphics_untextured_points = $prefs->nbody_graphics_untextured_points?"yes":"no";
    }

    if (NBODY_GRAPHICS_MONOCHROMATIC_PREF) {
        $prefs->nbody_graphics_monochromatic = parse_element($prefs_xml, "<nbody_graphics_monochromatic>");
        $prefs->nbody_graphics_monochromatic = $prefs->nbody_graphics_monochromatic?"yes":"no";
    }

    if (NBODY_GRAPHICS_ORIGIN_CENTERED_PREF) {
        $prefs->nbody_graphics_origin_centered = parse_element($prefs_xml, "<nbody_graphics_origin_centered>");
        $prefs->nbody_graphics_origin_centered = $prefs->nbody_graphics_origin_centered?"yes":"no";
    }

    if (NBODY_GRAPHICS_SHOW_INFO_PREF) {
        $prefs->nbody_graphics_show_info = parse_element($prefs_xml, "<nbody_graphics_show_info>");
        $prefs->nbody_graphics_show_info = $prefs->nbody_graphics_show_info?"yes":"no";
    }

    if (NBODY_GRAPHICS_SHOW_AXES_PREF) {
        $prefs->nbody_graphics_show_axes = parse_element($prefs_xml, "<nbody_graphics_show_axes>");
        $prefs->nbody_graphics_show_axes = $prefs->nbody_graphics_show_axes?"yes":"no";
    }

    if (NBODY_GRAPHICS_SHOW_ORBIT_TRACE_PREF) {
        $prefs->nbody_graphics_show_orbit_trace = parse_element($prefs_xml, "<nbody_graphics_show_orbit_trace>");
        $prefs->nbody_graphics_show_orbit_trace = $prefs->nbody_graphics_show_orbit_trace?"yes":"no";
    }

    return $prefs;
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
