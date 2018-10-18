/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "diagnose_usb.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <string>

#include <android-base/stringprintf.h>

#if defined(__linux__)
#include <grp.h>
#include <pwd.h>
#endif
#include <stdlib.h>
#include <limits.h>
#ifndef NGROUPS_MAX
#define NGROUPS_MAX        63        /* First guess.  */
#endif


/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
static int
__getgroups (int size, gid_t *list)
{
#if defined (NGROUPS_MAX) && NGROUPS_MAX == 0
  /* The system has no supplementary groups.  */
  return 0;
#endif
  //set_errno (ENOSYS);
  return -1;
}
//#if !(defined (NGROUPS_MAX) && NGROUPS_MAX == 0)
//stub_warning (getgroups);
//#endif

static int
group_member (gid_t gid)
{
  int n, size;
  gid_t *groups;
  size = NGROUPS_MAX;
  do
    {
      groups = (gid_t*)malloc(size * sizeof *groups);
      n = __getgroups (size, groups);
      size *= 2;
    }
  while (n == size / 2);
  while (n-- > 0)
    if (groups[n] == gid)
      return 1;
  return 0;
}


static const char kPermissionsHelpUrl[] = "http://developer.android.com/tools/device.html";

// Returns a message describing any potential problems we find with udev, or an empty string if we
// can't find plugdev information (i.e. udev is not installed).
static std::string GetUdevProblem() {
#if defined(__linux__)
    errno = 0;
    group* plugdev_group = getgrnam("plugdev");

    if (plugdev_group == nullptr) {
        if (errno != 0) {
            perror("failed to read plugdev group info");
        }
        // We can't give any generally useful advice here, just let the caller print the help URL.
        return "";
    }

    // getgroups(2) indicates that the GNU group_member(3) may not check the egid so we check it
    // additionally just to be sure.
    if (group_member(plugdev_group->gr_gid) || getegid() == plugdev_group->gr_gid) {
        // The user is in plugdev so the problem is likely with the udev rules.
        return "user in plugdev group; are your udev rules wrong?";
    }
    passwd* pwd = getpwuid(getuid());
    return android::base::StringPrintf("user %s is not in the plugdev group",
                                       pwd ? pwd->pw_name : "?");
#else
    return "";
#endif
}

// Short help text must be a single line, and will look something like:
//
//   no permissions (reason); see [URL]
std::string UsbNoPermissionsShortHelpText() {
    std::string help_text = "no permissions";

    std::string problem(GetUdevProblem());
    if (!problem.empty()) help_text += " (" + problem + ")";

    return android::base::StringPrintf("%s; see [%s]", help_text.c_str(), kPermissionsHelpUrl);
}

// Long help text can span multiple lines but doesn't currently provide more detailed information:
//
//   insufficient permissions for device: reason
//   See [URL] for more information
std::string UsbNoPermissionsLongHelpText() {
    std::string header = "insufficient permissions for device";

    std::string problem(GetUdevProblem());
    if (!problem.empty()) header += ": " + problem;

    return android::base::StringPrintf("%s\nSee [%s] for more information", header.c_str(),
                                       kPermissionsHelpUrl);
}
