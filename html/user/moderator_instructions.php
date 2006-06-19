<?php
require_once("../inc/util.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");

page_head("Moderator instructions");
echo "
Your job as a moderator is to enforce the following rules:
".post_rules()."
Delete posts or threads <b>only if they clearly violate
one or more of these rules</b>.
If in doubt, discuss it with other moderators on the email list.
Please respect the following guidelines:
<ul>
<li> Don't let your personal opinions
or moods affect your moderation decisions.
You may not delete a post simply because
you disagree with it or dislike its author.
If you find yourself getting angry, take a break.

<li> Don't discuss moderation decisions on the forums
of this or other BOINC projects.
Use the email list that has been set up for this purpose.

<li> Consider using an anonymous account for moderation,
so that your own forum activities are kept separate from
your moderation duties.

<li> Don't preemptively delete posts.
Except for obscene language or pictures,
avoid deleting a post until someone complains about it
(and, of course, don't delete it simply because someone
has complained).

</ul>
";
page_tail();

?>
