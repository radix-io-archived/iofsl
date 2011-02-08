#include <sstream>
#include <boost/format.hpp>
#include <cstring>
#include <boost/test/unit_test.hpp>
#include <boost/smart_ptr.hpp>

#include "iofwdutil/hash/HashFactory.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/hash/HashException.hh"


using namespace iofwdutil::hash;
using namespace boost;

/**
 * Static small tests
 */

static const char * md5_test[] = {
"0", "cfcd208495d565ef66e7dff9f98764da",
"1", "c4ca4238a0b923820dcc509a6f75849b",
"1.20GHz", "4aba5f82929b9d8a52e960fe8a762ef4",
"10", "d3d9446802a44259755d38e6d163e820",
"11", "6512bd43d9caa6e02c990b0a82652dca",
"1201.000", "01f561cd33b9f12ede4a2ebe226a4d9e",
"15", "9bf31c7ff062936a96d3c8bd1f8f2ff3",
"2", "c81e728d9d4c2f636f067f89cc14862c",
"2392.82", "46a634e20e6a481714cb43a84462b262",
"2393.77", "4d6ab803f7f3f1e471cfd776b3a2aa31",
"36", "19ca14e7ea6328a42e0eb13d585e4c22",
"4096", "f7efa4f864ae9b88d43527f4b14f750f",
"48", "642e92efb79421734881b53e1e1b18b6",
"6", "1679091c5a880faf6fb5e6087eb1b2dc",
"64", "ea5d2f1c4608232e07d3aa3d998e5135",
"@", "518ed29525738cebdac49c49e60ea9d3",
"CPU", "2b55387dd066c5bac646ac61543d152d",
"Core(TM)2", "44e31d1240d8b62d7ba78d9fd11da45c",
"Duo", "9d6ea08e871ed0e7206290f6b82173aa",
"GenuineIntel", "e435c43466900a489a98aed8bbfd6beb",
"Intel(R)", "cb55c612dbd0fdf1389b1350bba8e0c6",
"KB", "ab57fd0432e25d5b3013133a1c910d56",
"L7100", "546e58204f164d7096b158cd27cdfe7a",
"MHz:", "be98b83796f3d25b38f94176958f858b",
"acpi", "0512868916a7081afcfb789198de0f37",
"address", "884d9804999fc47a3c2694e49ad2536a",
"aperfmperf", "d92c1fd1faa18c6be697ed7aa689cb75",
"apic", "cf60064f9f54d08844113c5d696a1a9c",
"apicid:", "b4c0294c523dd20310fccc3712936a28",
"apicid:", "b4c0294c523dd20310fccc3712936a28",
"arch_perfmon", "ab5acc202e9c095a400db4bc32a72a50",
"bits", "cc411e6c13670e52124629b8ac83f7d0",
"bogomips:", "f1190790f048835deadd127f9259af57",
"bts", "c3ea886e7d47f5c49a7d092fadf0c03b",
"cache", "0fea6a13c52b4d4725368f24b045ca84",
"cache_alignment:", "66f3f210042c0bd54a84060a5f90e586",
"clflush", "dc1a1dbdcd56dbb476f15d7bf99f3905",
"cmov", "4d83d8b75d75dfbb6a9bf86d38c12a22",
"constant_tsc", "c3a146ca59fb8b1151287766d1057628",
"core", "a74ad8dfacd4f985eb3977517615ce25",
"cores:", "8976258cf496b55da4dd0935a7aa81bc",
"cpu", "d9747e2da342bdb995f6389533ad1a3d",
"cpuid", "e5a1ef0ecd3d12276e8833163f21d267",
"cx16", "157dd4f584be2910d2d161bb3df90479",
"cx8", "5eabcd73ed254393ddcd7adb7455c548",
"de", "5f02f0889301fd7be1ac972c11bf3e7d",
"ds_cpl", "0690c1fa3f4baef35b2cd4b79f9a042e",
"dtes64", "d79763b167141ab2ae870731a66f334f",
"dts", "da358a1216cf11bd724c0aa17441a5d2",
"est", "1c52bdae8bad70e82da799843bb4e831",
"family:", "8df7bcc0865ed0955eb2521d8c35e576",
"flags:", "0f057d62a8e6312ad3a61813e60802fa",
"flexpriority", "731dfd6fc2e646bdf8385113eef9521b",
"fpu", "0075ab5617d7f0d01c017e92e8a46e22",
"fpu:", "2e67b9c0a9c8ede6c4367f4625bcf2b1",
"fpu_exception:", "dd23d3d049b7286d9b355e84ea9e0c72",
"fxsr", "7491a2b0ae270956a0f093f2604a09fd",
"ht", "eb5e48e74123cacc52761302ea4a7d22",
"id:", "db2f7cf2f8d7bbec9222372327e4bed5",
"id:", "db2f7cf2f8d7bbec9222372327e4bed5",
"ida", "7f78f270e3e1129faf118ed92fdf54db",
"initial", "cc51b81974287ab79cef9e94fe778cc9",
"lahf_lm", "2a7cb6684256a92139c09061aae230f9",
"level:", "8213a5e6fd8fe582a74f6afa733ba8ef",
"lm", "192292e35fbe73f6d2b8d96bd1b6697d",
"management:", "c5dc43f39e87c21c31ae19cc2e0b78cb",
"mca", "7b8a3544637ef0e8fd2859095876e461",
"mce", "c5f6e3479d45dbb807d46852a6a3b077",
"mmx", "c1cdb8620588857c5b18f05191f45bb2",
"model", "20f35e630daf44dbfa4c3f68f5399d8c",
"model:", "e1a0d092565ece1bb928021f006fa026",
"monitor", "08b5411f848a2581a41672a759c87380",
"msr", "38b33779833838a98c2a241ce465fb07",
"mtrr", "b95750d137948788b3832f39bbe2deac",
"name:", "d942692a8122af8eb3b3adc3cc48a614",
"nx", "97893f46e7e13ef37b4c2e0ac60d85ca",
"pae", "6d2d25cac6ce5b576c4509e535e4d3d4",
"pat", "7852341745c93238222a65a910d1dcc5",
"pbe", "015704ef80eacdde3751702f30f4d980",
"pdcm", "90d623af211387b3dc8ea32938faab03",
"pebs", "1b6adfee6db396910f911d0f1fdc3fba",
"pge", "57b79e0c137349ec8176add14e947652",
"physical", "842e5a39b01fadaead6970f5ff5de1d5",
"physical,", "273148876c58da655309d21b483b8bab",
"pni", "b92623f78d1b691a8f3ecd2ac176ceb4",
"power", "62cd275989e78ee56a81f0265a87562e",
"processor:", "9ec1b35513fc9a45f303bbedfa3375e6",
"pse", "30ea0a768cc9d95255e241efe585e23a",
"pse36", "cf7c96b3df44426c2626f7ca8c52f753",
"rep_good", "c483221ef3ba203edb2e6961e0a68b3e",
"sep", "314e9e118b3026ce64b768b84a22d816",
"siblings:", "0628cddb22818ae0feb4350736c660a4",
"size:", "e72052f88af384b91c675257747e3e74",
"sizes:", "c8398e6b5f3ae3916138e44f0004071d",
"ss", "3691308f2a4c2f6983f2880d32e29c84",
"sse", "64192ca465194480a4621d6905dac5b7",
"sse2", "0303bb014717eba9fdeae96c3629fe5f",
"ssse3", "eb8f495b5d7e93ecace5820749c2dec2",
"stepping:", "7e3ddf6789af70e293188e36764dd55b",
"syscall", "620d9e52985d394f7b1a12db6b2d4e56",
"tm", "6a962563e235e1789e663e356ac8d9e4",
"tm2", "76a6c2f5d880a1f93ab07844e96a3e6b",
"tpr_shadow", "3062a10811a7bca39dc1c22639071bb2",
"tsc", "3c33894421ec07809347e7b0b7b19359",
"vendor_id:", "e0bace6c9d58bfaaaa9d1df290392280",
"virtual", "db8857b420665169e36ad60c8dfc23a5",
"vme", "a53a5e4d23a31714c273bf4371a86370",
"vmx", "6c096a5ac5041d94699db9c11917cc22",
"vnmi", "d9805be080705735b1b65ad4c488dc91",
"wp:", "ce8316a1f2657ab137a8df82ada2882c",
"xtpr", "bc30d87ace0a3d4c76c29177e544b89f",
"yes", "a6105c0a611b41b08f1209506350279e",
0,0
};

static const char * sha1_test[] = {
"0", "b6589fc6ab0dc82cf12099d1c2d40ab994e8410c",
"1", "356a192b7913b04c54574d18c28d46e6395428ab",
"1.20GHz", "a8e2c9dd56930395f8bcf7a2305c77fc58cf900d",
"10", "b1d5781111d84f7b3fe45a0852e59758cd7a87e5",
"11", "17ba0791499db908433b80f37c5fbc89b870084b",
"1201.000", "98f96938b07786bfe82303a355a7725ea5712de8",
"15", "f1abd670358e036c31296e66b3b66c382ac00812",
"2", "da4b9237bacccdf19c0760cab7aec4a8359010b0",
"2392.82", "7c8a171d5a1b8616fee11980f501f6d4a5018eb5",
"2393.77", "513ce96b5013eb4fa3076f2448eb5e20eed26526",
"36", "fc074d501302eb2b93e2554793fcaf50b3bf7291",
"4096", "6124cb94c33db249c29395625c4b43deb8be0cb6",
"48", "64e095fe763fc62418378753f9402623bea9e227",
"6", "c1dfd96eea8cc2b62785275bca38ac261256e278",
"64", "c66c65175fecc3103b3b587be9b5b230889c8628",
"@", "9a78211436f6d425ec38f5c4e02270801f3524f8",
"CPU", "ff221d4752ce05f5a91bbf1d28b78a7bf7e2ddaa",
"Core(TM)2", "32fb027b0da5dc2b9f671b915a18ee752eb7da23",
"Duo", "3c5d9cd03387fb41a856eb94a3f12c1e7c45d2c1",
"GenuineIntel", "d0b1ed23a8ebaad9a7957d3b8148839a0c6fd236",
"Intel(R)", "722c0ea4c6e55c1ecab58d8725e65af8c5302747",
"KB", "315b39a0fc19fb45d146c17b813d46c7acb0b872",
"L7100", "6e98755273e768fdec4884b61fd287603540510c",
"MHz:", "87d66acb101eac3ccc7a5d821901f501ce4ddaac",
"acpi", "bbde8cad439ba949c222a56414f004b0dca5899c",
"address", "c662180230cad14787d4ab7e77aa08681ce783fa",
"aperfmperf", "f7434e01a750f909bdddbd8f62def0a0abdb47ea",
"apic", "4e5eb874091dbbe04f24c282751a788b3bf0f163",
"apicid:", "2d56851145a0aec8c6e6aec18eb020898803587f",
"apicid:", "2d56851145a0aec8c6e6aec18eb020898803587f",
"arch_perfmon", "f13f58237987d039aebd22dad1b20841ff99e205",
"bits", "0569ae912e387dbc988dee842bf17cdd4e437f63",
"bogomips:", "03226e51f57caeaa1916184bb74680d145c31e0e",
"bts", "f6ff067421fbd90571240adea4e50eb715e49ac0",
"cache", "b03592806efabfeeb709f5a70a7c172669b00538",
"cache_alignment:", "440ada8e962b55a0cb04bf0f18a4a9fe0c26ad0a",
"clflush", "d454892084827ce17e61e0e40efa02f9a0ac63e0",
"cmov", "12df0b00d359815111ae68813bd63291a69c9323",
"constant_tsc", "272f405fc48408981ac8751bde0f1c1d41d8a653",
"core", "94a0426e8d3203da5468ccf0c624f93cb37601e2",
"cores:", "ca344747ff9b5d409c4532552b9290c6d9d7d202",
"cpu", "ca49ca4bb6732ef61577364cb33b115854e88f06",
"cpuid", "5e90890baa20c3ca8da01d893b42f95b88f6003f",
"cx16", "50386b4a93bc1619048971efe68a9569d2d10d50",
"cx8", "02bce945626d7a20954ecae69c335f7332c41179",
"de", "600ccd1b71569232d01d110bc63e906beab04d8c",
"ds_cpl", "aa2a26dff058c6129a7cb0941605b079e9f90b94",
"dtes64", "e72031727d6a625de5622afa9f6fcc67ab906ff6",
"dts", "ea17922c53f89cdf15cadf474395eb990c4c9954",
"est", "665de1f2775ca0b64d3ceda7c1b4bd15e32a73ed",
"family:", "9dfdff20e3e3e1dd8724770922b0c48790c268d0",
"flags:", "649adc91552d9200989ef4536551bac064614ef7",
"flexpriority", "cd41f8c23a77f4d9cd5eaf4339abffdf482b224c",
"fpu", "8e8833b971984570d5609eddfbcf9648faac1534",
"fpu:", "312f7bff783b38bece2ffa2bfa4bbcf477198a5d",
"fpu_exception:", "5293c5f4634d20c17cb2679609e11a76b0a902ac",
"fxsr", "d90ffe5b3ce864fc23d19b66e286d2c301623bf0",
"ht", "80f6b1db5d2c25e15a77836728a708218a092cbc",
"id:", "a078622f8db42bfd32578d9f982b420aca195a41",
"id:", "a078622f8db42bfd32578d9f982b420aca195a41",
"ida", "7317d52c532799ddea1df342c1a159c05deb925d",
"initial", "8ad7d21c71b049b7003ba31b5f1322974df77ac8",
"lahf_lm", "d33ffa971fd537b32322e683235a70757e335317",
"level:", "743e8359279cf7f0547f8ae5b735a7cf348aa31e",
"lm", "93ed4a0a3d0e1873efad9e42341ac36b31958df6",
"management:", "6906fadd8716e0479be2845651aa033760bcf71e",
"mca", "5f77c9a9df5a7449cd044610cbe6e3be4647f197",
"mce", "c60d974101992c286b27fc6d07911134d2104b62",
"mmx", "4c2c3df089fdbb10bf15f7da93f890a8f1c84861",
"model", "1d06a0d76f000e6edd18de492383983feefced4e",
"model:", "8a37db6ef77e02d0b01702ab284233b3128ca11b",
"monitor", "9796809f7dae482d3123c16585f2b60f97407796",
"msr", "b7cdecd2f3449609e17d97d62dbc330fa800b77c",
"mtrr", "01b0600677d51db8c1177b34d8fdbd321a7eb886",
"name:", "092787538b7654f8c0ee70683508b96328605e8b",
"nx", "9c93b4599fe3df75fb3326420f7805a0b572eb98",
"pae", "c78347c0ee5686b8c22ba72f5674394bd9ef26c8",
"pat", "7ffe1ba40f2584b96991dec44ac44fe7e8d6bc68",
"pbe", "2ce3901b3eea891075a680dd803fe77f747a88ff",
"pdcm", "da778bc5ec60c009242361136d0ff414eb8b90b2",
"pebs", "5c7a01593ce5b86e9b140679012207c39de47f6e",
"pge", "dbf662a314870538bf3e6396f6217fc816f6ed45",
"physical", "aa8d71087485ba458fb29817943309e50cf31ba8",
"physical,", "3aa27c8ae793ae0fe6df61d90f61ba1a638edd94",
"pni", "874f89e02e1476cc8e53922471c8e8f95eed3293",
"power", "b573f24e55d6b7547cb53bd67b8f50a5256006ff",
"processor:", "a235e4be0fe79c4554bb055db6d72cc6342e081f",
"pse", "bd2df6c6df2bd72a8bbb7aea33a313cd4e9186ec",
"pse36", "fbf21d5ffd4a789dfa2c088c01acd223f967ae46",
"rep_good", "c497f630a05b9a5f158831570da1abaea22a85b8",
"sep", "9162e8705ede492c514e765f7ed3474d85922ff4",
"siblings:", "ab2d9e27ee26546609ec05f489f62bded8d76693",
"size:", "8bce0c42ce9e4fa462e7dba090b3646330b583ac",
"sizes:", "548d1794e571d7b18fbecaed8611252790d0f3e1",
"ss", "c1c93f88d273660be5358cd4ee2df2c2f3f0e8e7",
"sse", "57ab81fe5dce1359b511894adb9c0ebfb25e3327",
"sse2", "97f6692c6ba548d214597d9313d19e81a390f750",
"ssse3", "4c86cdb6e50c1a9f5267ecc8a12650440a8e8055",
"stepping:", "f797972d097be8787efbf03ebe2fd5c1540d7a2d",
"syscall", "d7cbea74df93b67d6611b9ce87cf4f9a6b0c258f",
"tm", "a547db31280d14f095c760a23a7523d0b1e55fe6",
"tm2", "3aafc06dc02aecfcafd5b8d9d2e378de80a41aca",
"tpr_shadow", "073b52b537a74da80dabdd89ff9fdd3111faa1a6",
"tsc", "50979f87c66719e3509a3b862559b255398772bb",
"vendor_id:", "ce52590733c74ef4e2edf2f00497fd6e7cdc92f2",
"virtual", "af84d91fde168566c7dc18f3121ea2fbe651af1f",
"vme", "96a99b1e99e653e8501b5dfd4b6eb71d43f89e0e",
"vmx", "1622e14385bdbb70450a54259fdaec1f48e232fc",
"vnmi", "206ca31e17e7239780cd44c5b0521d84d10d7b15",
"wp:", "6307975b27ed7303e255f37054f1dee33ad16fbe",
"xtpr", "fe4d43c9964e8bcede9e20d8d3c666bb31ba6185",
"yes", "fb360f9c09ac8c5edb2f18be5de4e80ea4c430d0",
0, 0
};

static const char * none_test[] = 
{
   "dfdf", "",
   "", "",
   0, 0
};

struct F
{
   F ()
      : factory_(iofwdutil::hash::HashFactory::instance ())
   {
   }

   HashFactory & factory_;
};

// @TODO: add tests for longer data strings (for example files stored in repo)
// @TODO: test final parameter on process
// @TODO: check that function throws for hashes that don't exist

std::string toHex (const char * buf, size_t bufsize)
{
   std::ostringstream out;
   for (size_t i=0; i<bufsize; ++i)
   {
      out << str(format("%02x") % (unsigned) (unsigned char) buf[i]);
   }
   return out.str();
}

void checkOut (const char * ok, const char * buf, size_t bufsize)
{
   BOOST_CHECK_EQUAL (std::string(ok), toHex (buf, bufsize));
}

void testFunc (HashFunc * hash, const char ** data)
{
   BOOST_TEST_MESSAGE(format("Testing hash func '%s'") % hash->getName());
   const char ** ptr = data;
   const size_t hashlen = hash->getHashSize();
   BOOST_TEST_MESSAGE(format("Hash length: %i bytes") % hashlen);
   boost::scoped_array<char> buf (new char[hashlen]);
   while (*ptr)
   {
      const char * input = *ptr++;
      const char * output = *ptr++;
      ALWAYS_ASSERT(strlen(output) % 2 == 0);

      for (size_t count =0; count<2; ++count)
      {
         hash->process (input, strlen(input));
         const size_t outcount = hash->getHash (buf.get(), hashlen, true);
         BOOST_CHECK_EQUAL (outcount, hashlen);
         checkOut (output, buf.get(), hashlen);
         hash->reset ();
      }
   }
}


void doTest (HashFactory & factory, const char * name, const char ** data)
{
   BOOST_TEST_MESSAGE(format("Getting hash functions for '%s'") % name);
   std::vector<HashFunc *> funcs;
   try
   {
      factory.getHash (name, funcs);
   }
   catch (NoSuchHashException & e)
   {
      BOOST_TEST_MESSAGE("Skipping test - could not find hash function");
   }

   try
   {
      for (size_t i=0; i<funcs.size(); ++i)
      {
         testFunc (funcs[i], data);
      }
   }
   catch (...)
   {
      // cleanup
      for (size_t i=0; i<funcs.size(); ++i)
      {
         delete (funcs[i]);
      }
      throw;
   }
}


BOOST_FIXTURE_TEST_CASE ( factory, F )
{
   BOOST_TEST_MESSAGE(format("Factory has %i functions defined...") %
         factory_.size());
   doTest (factory_, "md5", md5_test);
   doTest (factory_, "sha1", sha1_test);
   doTest (factory_, "none", none_test);
}
