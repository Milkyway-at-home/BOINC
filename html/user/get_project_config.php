<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

xml_header();

// This all needs to work even when DB is down.
// So cache list of platforms in a file,
// and update it every hour if possible.
//
function show_platforms() {
    $path = "../cache/platform_list.xml";
    $mtime = @filemtime($path);
    if ($mtime && (time() - $mtime) < 3600) {
        @readfile($path);
    } else {
        require_once("../inc/db.inc");
        $retval = db_init_aux(true);
        if (!$retval) {
            $query = 'select name, user_friendly_name from platform, app_version where app_version.platformid = platform.id and app_version.deprecated=0 group by name';
            $result = mysql_query($query);
            $f = fopen($path, "w");
            fwrite($f, "<platforms>\n");
            while ($p = mysql_fetch_object($result)) {
                fwrite($f,
                    "  <platform>\n    <platform_name>$p->name</platform_name>\n    <user_friendly_name>$p->user_friendly_name</user_friendly_name>\n  </platform>\n"
                );
            }
            mysql_free_result($result);
            fwrite($f, "</platforms>\n");
            fclose($f);
            @readfile($path);
        }
    }
}

$config = get_config();
$long_name = parse_config($config, "<long_name>");

// if project has a min client version, enforce it
//
$min_core_client_version = parse_config($config, "<min_core_client_version>");
if ($min_core_client_version) {
    $x = $_SERVER['HTTP_USER_AGENT'];
    list($platform, $maj, $min)  = sscanf($x, "BOINC client (%s %d.%d)");
    $too_old = false;
    if ($maj !== null && $min !== null) {
        $v = $maj*100 + $min;
        if ($v < $min_core_client_version) {
            $too_old = true;
        }
    } else {
        $too_old = true;
    }
    if ($too_old) {
        $rmaj = $min_core_client_version/100;
        $rmin = $min_core_client_version%100;
        echo "<error_msg>This project requires BOINC client version $rmaj.$rmin or later.</error_msg>
<error_num>-190</error_num>
</project_config>
";
        exit;
    }
}

$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) {
    $min_passwd_length = 6;
}
$disable_account_creation = parse_bool($config, "disable_account_creation");
$master_url = parse_config($config, "<master_url>");

echo "<project_config>
    <name>$long_name</name>
    <master_url>$master_url</master_url>
";

$local_revision = trim(file_get_contents("../../local.revision"));
if ($local_revision) {
    echo "<local_revision>$local_revision</local_revision>\n";
}

if (web_stopped()) {
    echo "
        <error_num>-183</error_num>
        <web_stopped>1</web_stopped>
    ";
} else {
    echo "<web_stopped>0</web_stopped>\n";
    if ($disable_account_creation || defined('INVITE_CODES')) {
        echo "    <account_creation_disabled/>\n";
    }
    echo "
        <min_passwd_length>$min_passwd_length</min_passwd_length>
    ";
}
if (sched_stopped()) {
    echo "<sched_stopped>1</sched_stopped>\n";
} else {
    echo "<sched_stopped>0</sched_stopped>\n";
}

show_platforms();

$terms_of_use = trim(file_get_contents("../../terms_of_use.txt"));
if ($terms_of_use) {
    echo "<terms_of_use>\n$terms_of_use\n</terms_of_use>\n";
}

echo "
</project_config>
";

?>
