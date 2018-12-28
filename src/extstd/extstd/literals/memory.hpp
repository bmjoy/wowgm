#pragma once

constexpr size_t operator"" _KB(size_t kilobytes) { return kilobytes * 1024u; }
constexpr size_t operator"" _MB(size_t megabytes) { return megabytes * 1024u * 1024u; }
constexpr size_t operator"" _GB(size_t gigabytes) { return gigabytes * 1024u * 1024u * 1024u; }

constexpr size_t operator"" _KiB(size_t kilobytes) { return kilobytes * 1000u; }
constexpr size_t operator"" _MiB(size_t megabytes) { return megabytes * 1000u * 1000u; }
constexpr size_t operator"" _GiB(size_t gigabytes) { return gigabytes * 1000u * 1000u * 1000u; }
