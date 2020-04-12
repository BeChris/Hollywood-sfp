#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <hollywood/plugin.h>

#include "sfpplugin.h"
#include "version.h"

extern hwPluginAPI *hwcl;

extern void add_entry(void *state, const char *key, const char *value);

void fill_systable(void *state)
{
	APTR handle = hw_Lock("/sys/devices/virtual/dmi/id/", HWLOCKMODE_READ);

	if (handle != NULL)
	{
		APTR dirhandle;
		int error = hw_BeginDirScan(handle, &dirhandle);

		if (error == 0)
		{
			int ok = FALSE;

			for (;;)
			{
				struct hwos_ExLockStruct exlock;
				exlock.Size = sizeof(struct hwos_ExLockStruct);

				ok = hw_NextDirEntry(handle, dirhandle, &exlock);

				if (ok == FALSE)
				{
					break;
				}

				if (exlock.Type == HWEXLOCKTYPE_FILE)
				{
					//printf("DirEntry : %s\n", exlock.Name);
					unsigned char fullname[1024] = "/sys/devices/virtual/dmi/id/";
					ok = hw_AddPart(fullname, exlock.Name, 1024);

					if (ok == TRUE)
					{
						//printf("FullName : %s\n", fullname);

						APTR rd = hw_FOpen(fullname, HWFOPENMODE_READ_NEW);

						if (rd != NULL)
						{
							//printf("Opened file\n");

							struct hwos_StatStruct st;
							if (hw_FStat(rd, 0, &st, NULL) == TRUE)
							{
								//printf("FStat OK : size=%lld\n", st.Size);

								if (st.Size > 0)
								{
									char *content = malloc(st.Size * sizeof(char));
									memset(content, 0, st.Size * sizeof(char));

									int read = hw_FRead(rd, content, st.Size);

									if (read > 1)
									{
										//printf("FRead OK : read=%d\n", read);

										int idx = strlen(content) - 1;

										while (idx > 0)
										{
											if (content[idx] == '\n' || content[idx] == '\r')
											{
												content[idx] = '\0';
											}

											--idx;
										}

										//printf("FRead content : %s\n", content);
                                        add_entry(state, exlock.Name, content);

									}

									free(content);
								}

							}

							hw_FClose(rd);
						}
					}
				}
			}

			hw_EndDirScan(dirhandle);
		}

		hw_UnLock(handle);

	}


}