#include "sergut/Version.h"

namespace sergut {
Version Version::getCurrentVersion()
{
  return Version(1, 1, 0, "SNAPSHOT");
}

}
