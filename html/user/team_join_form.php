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

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user();
$teamid = get_int("id");

$team = lookup_team($teamid);
if (!$team->joinable) {
    error_page("The team is not joinable.");
}
$team_name = $team->name;
page_head("Join $team_name");
echo " <p><b>Please note:</b>
    <ul>
    <li> Joining a team gives its founder access to your email address.
    <li> Joining a team does not affect your account's credit.
    </ul>
    <hr>
    <form method=\"post\" action=\"team_join_action.php\">";
echo form_tokens($user->authenticator);
echo "
    <input type=\"hidden\" name=\"teamid\" value=\"$teamid\">
    <input type=\"submit\" value=\"Join team\">
    </form>
";
page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
