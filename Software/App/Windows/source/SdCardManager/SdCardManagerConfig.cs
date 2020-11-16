
namespace SdCardManager
{
    public class SdCardManagerConfig
    {
        public ushort PasswordLength { get; set; } = 80;
        public string ValidChars { get; set; } = "abcdefghijklmnopqrstuvwxABCDEFGHIJKLMNOPQRSTUVWX1234567890";
        public string CharsMustContainAtLeastOne { get; set; } = "!%&";
        public bool IsAdvanced { get; set; }
        public bool IsShowPassword { get; set; }
    }
}
