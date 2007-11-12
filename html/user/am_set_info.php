<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");

// do a very cursory check that the given text is valid;
// for now, just make sure it has the given start and end tags,
// and at least one \n in the middle.
// Ideally, we'd like to check that it's valid XML
//
function bad_xml($text, $start, $end) {
    $text = trim($text);
    if (strstr($text, $start) != $text) {
        return "No start tag";
    }
    if (strstr($text, $end) != $end) {
        return "No end tag";
    }
    if (!strstr($text, "\n")) {
        return "No CR";
    }
    return "";
}

function success($x) {
    echo "<am_set_info_reply>
    <success/>
    $x
</am_set_info_reply>
";
}


xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$auth = process_user_text($_GET["account_key"]);
$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
}

$name = process_user_text($_GET["name"]);
$country = $_GET["country"];
if ($country && !is_valid_country($country)) {
    xml_error(-1, "invalid country");
}
$postal_code = process_user_text($_GET["postal_code"]);
$global_prefs = process_user_text($_GET["global_prefs"]);
$project_prefs = process_user_text($_GET["project_prefs"]);

// Do processing on project prefs so that we don't overwrite project-specific
// settings if AMS has no idea about them

if (stripos($project_prefs, "<project_specific>") === false) {
    // AMS request does not contain project specific prefs, preserve original
    $orig_project_specific = stristr($user->project_prefs, "<project_specific>");
    $orig_project_specific = substr($orig_project_specific, 0, stripos($orig_project_specific, "</project_specific>") + 19)."\n";
    $project_prefs = str_ireplace("<project_preferences>", "<project_preferences>\n".$orig_project_specific, $project_prefs);
}

$url = process_user_text($_GET["url"]);
$send_email = process_user_text($_GET["send_email"]);
$show_hosts = process_user_text($_GET["show_hosts"]);
$teamid = get_int("teamid", true);
$venue = process_user_text($_GET["venue"]);
$email_addr = strtolower(process_user_text($_GET["email_addr"]));
$password_hash = process_user_text($_GET["password_hash"]);

$query = "";
if ($name) {
    $query .= " name='$name', ";
}
if ($country) {
    $query .= " country='$country', ";
}
if ($postal_code) {
    $query .= " postal_code='$postal_code', ";
}
if ($global_prefs) {
    $global_prefs = str_replace("\\r\\n", "\n", $global_prefs);
    $x = bad_xml($global_prefs, "<global_preferences>", "</global_preferences>");
    if ($x) {
        error("Invalid global preferences: $x");
    }
    $query .= " global_prefs='$global_prefs', ";
}
if ($project_prefs) {
    $project_prefs = str_replace("\\r\\n", "\n", $project_prefs);
    $x = bad_xml($project_prefs, "<project_preferences>", "</project_preferences>");
    if ($x) {
        xml_error(-112, "Invalid project preferences: $x");
    }
    $query .= " project_prefs='$project_prefs', ";
}
if ($url) {
    $query .= " url='$url', ";
}
if ($send_email != null) {
    $query .= " send_email='$send_email', ";
}
if ($show_hosts != null) {
    $query .= " show_hosts='$show_hosts', ";
}

if (!is_null($teamid)) {
    if ($teamid==0) {
        user_quit_team($user);
    } else {
        $team = lookup_team($teamid);
        if ($team) {
            user_join_team($team, $user);
        }
    }
}

if ($venue) {
    $query .= " venue='$venue', ";
}
if ($email_addr && $email_addr!=$user->email_addr) {
    $old_email_addr = $user->email_addr;
    $query .= " email_addr='$email_addr', ";
}
if ($password_hash) {
    $query .= " passwd_hash='$password_hash', ";
}

if (strlen($query)) {
    // the seti_id=seti_id is to make the query valid,
    // since $query ends with a comma at this point
    //
    $query = "$query seti_id=seti_id";
    $result = $user->update($query);
    if ($result) {
        success("");
    } else {
        xml_error(-1, "database error: ".BoincDb::error());
    }
} else {
    success("");
}

?>
