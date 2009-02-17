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

db_init();

$user = get_logged_in_user(false);

page_head("Change password");

echo "
    <form method=post action=edit_passwd_action.php>
";


if ($user) {
    echo "
        <input type=hidden name=auth value=$user->authenticator>
    ";
    start_table();
} else {
    start_table();
    row1("You can identify yourself using either
        <ul>
        <li> your email address and old password
        <li> your account key
        </ul>
        "
    );
    row2("Email address", "<input name=email_addr size=40>");
    row2("Current password", "<input type=password name=old_passwd size=40>");
    row2(
        "<b>OR</b>: Account key
        <br><font size=-2><a href=get_passwd.php>Get account key by email</a>",
        "<input name=auth size=40>"
    );
}
row2("New password", "<input type=password name=passwd size=40>");
row2("New password, again", "<input type=password name=passwd2 size=40>");
row2("", "<input type=submit value='Change password'>");
end_table();
echo "</form>\n";
page_tail();
?>
