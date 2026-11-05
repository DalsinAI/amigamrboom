/* Mr.Boom's game logic (mrboom.c, translated from the DOS original's
 * assembly) keeps its registers and call stack inside its Memory struct,
 * but program() is one very large function and the AI's behaviour tree
 * recurses a little; libnix's startup sets up this much stack instead of
 * a Shell's 4 KB. The AmiCradle Team. MIT, like Mr.Boom. */
unsigned long __stack = 256 * 1024;
