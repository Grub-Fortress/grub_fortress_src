// This is a script to test the downloads table functions

local tempfilename = "bf/test.txt"
StringToFile("test.txt", "Hello World!")

local filename = "scriptdata/" + tempfilename
local result = AddFileToDownloadsTable(filename)
local downloadslength = GetDownloadsTableLength()

printf("%s '%s' to the downloads table\n", result ? "Successfully Added" : "Failed to add", filename)
printf("'%s' %s the downloads table\n", filename, IsFileInDownloadsTable(filename) ? "is in" : "is not in")
printf("The downloads table length is %i\n", downloadslength)

for(local i = 0; i < downloadslength; i++)
{
	printf("file: %s\n", GetStringFromDownloadsTable(i))
}

// The code below does not function as intended.
/*
result = RemoveFileFromDownloadsTable(filename)
printf("%s '%s' from the downloads table\n", result ? "Successfully Removed" : "Failed to remove", filename)

downloadslength = GetDownloadsTableLength()
for(local i = 0; i < downloadslength; i++)
{
	printf("file: %s\n", GetStringFromDownloadsTable(i))
}
*/