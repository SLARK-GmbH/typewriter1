
using System;

namespace SdCardManager
{
    public class FileEntry
    {
        public string DisplayName { get; set; }
        public string FileType { get; set; }
        public string Age { get; set; }
        public long Length { get; set; }
        public string Content { get; set; }
        public string FullPath { get; set; }
    }
}
