/**
 * ***README***
 * It was pointed out in #58 that the existing property-related utilities
 * like get_writer/has_reader... are slow (current version is v0.12.1).
 *
 * This benchmark should help diagnose and (try to) optimise these routines.
 */
#define REFL_DISALLOW_SEARCH_FOR_RW
#include "refl.hpp"

template<class T>
void ProcessClass()
{
   constexpr auto type = refl::reflect<T>();
   constexpr auto members = get_members(type);

   for_each(members, [&](auto member)
   {
      if constexpr (refl::descriptor::is_property(member))
      {
         if constexpr (refl::descriptor::is_writable(member))
         {
            auto reader = refl::descriptor::get_reader(member);
            // etc.
         }
         else if constexpr (! refl::descriptor::has_writer(member))
         {
            // etc.
         }
      }
   });
}

// The class below has 100 reflected properties. Define PROPERTIES_2X
// to define 100 more properties (double the number of properties). Useful for
// benchmarking.
// #define PROPERTIES_2X

class LargePod
{
public:
    int GetItem0() const { return 0; }
    void SetItem0(int) {}
    int GetItem1() const { return 0; }
    void SetItem1(int) {}
    int GetItem2() const { return 0; }
    void SetItem2(int) {}
    int GetItem3() const { return 0; }
    void SetItem3(int) {}
    int GetItem4() const { return 0; }
    void SetItem4(int) {}
    int GetItem5() const { return 0; }
    void SetItem5(int) {}
    int GetItem6() const { return 0; }
    void SetItem6(int) {}
    int GetItem7() const { return 0; }
    void SetItem7(int) {}
    int GetItem8() const { return 0; }
    void SetItem8(int) {}
    int GetItem9() const { return 0; }
    void SetItem9(int) {}
    int GetItem10() const { return 0; }
    void SetItem10(int) {}
    int GetItem11() const { return 0; }
    void SetItem11(int) {}
    int GetItem12() const { return 0; }
    void SetItem12(int) {}
    int GetItem13() const { return 0; }
    void SetItem13(int) {}
    int GetItem14() const { return 0; }
    void SetItem14(int) {}
    int GetItem15() const { return 0; }
    void SetItem15(int) {}
    int GetItem16() const { return 0; }
    void SetItem16(int) {}
    int GetItem17() const { return 0; }
    void SetItem17(int) {}
    int GetItem18() const { return 0; }
    void SetItem18(int) {}
    int GetItem19() const { return 0; }
    void SetItem19(int) {}
    int GetItem20() const { return 0; }
    void SetItem20(int) {}
    int GetItem21() const { return 0; }
    void SetItem21(int) {}
    int GetItem22() const { return 0; }
    void SetItem22(int) {}
    int GetItem23() const { return 0; }
    void SetItem23(int) {}
    int GetItem24() const { return 0; }
    void SetItem24(int) {}
    int GetItem25() const { return 0; }
    void SetItem25(int) {}
    int GetItem26() const { return 0; }
    void SetItem26(int) {}
    int GetItem27() const { return 0; }
    void SetItem27(int) {}
    int GetItem28() const { return 0; }
    void SetItem28(int) {}
    int GetItem29() const { return 0; }
    void SetItem29(int) {}
    int GetItem30() const { return 0; }
    void SetItem30(int) {}
    int GetItem31() const { return 0; }
    void SetItem31(int) {}
    int GetItem32() const { return 0; }
    void SetItem32(int) {}
    int GetItem33() const { return 0; }
    void SetItem33(int) {}
    int GetItem34() const { return 0; }
    void SetItem34(int) {}
    int GetItem35() const { return 0; }
    void SetItem35(int) {}
    int GetItem36() const { return 0; }
    void SetItem36(int) {}
    int GetItem37() const { return 0; }
    void SetItem37(int) {}
    int GetItem38() const { return 0; }
    void SetItem38(int) {}
    int GetItem39() const { return 0; }
    void SetItem39(int) {}
    int GetItem40() const { return 0; }
    void SetItem40(int) {}
    int GetItem41() const { return 0; }
    void SetItem41(int) {}
    int GetItem42() const { return 0; }
    void SetItem42(int) {}
    int GetItem43() const { return 0; }
    void SetItem43(int) {}
    int GetItem44() const { return 0; }
    void SetItem44(int) {}
    int GetItem45() const { return 0; }
    void SetItem45(int) {}
    int GetItem46() const { return 0; }
    void SetItem46(int) {}
    int GetItem47() const { return 0; }
    void SetItem47(int) {}
    int GetItem48() const { return 0; }
    void SetItem48(int) {}
    int GetItem49() const { return 0; }
    void SetItem49(int) {}
    int GetItem50() const { return 0; }
    void SetItem50(int) {}
    int GetItem51() const { return 0; }
    void SetItem51(int) {}
    int GetItem52() const { return 0; }
    void SetItem52(int) {}
    int GetItem53() const { return 0; }
    void SetItem53(int) {}
    int GetItem54() const { return 0; }
    void SetItem54(int) {}
    int GetItem55() const { return 0; }
    void SetItem55(int) {}
    int GetItem56() const { return 0; }
    void SetItem56(int) {}
    int GetItem57() const { return 0; }
    void SetItem57(int) {}
    int GetItem58() const { return 0; }
    void SetItem58(int) {}
    int GetItem59() const { return 0; }
    void SetItem59(int) {}
    int GetItem60() const { return 0; }
    void SetItem60(int) {}
    int GetItem61() const { return 0; }
    void SetItem61(int) {}
    int GetItem62() const { return 0; }
    void SetItem62(int) {}
    int GetItem63() const { return 0; }
    void SetItem63(int) {}
    int GetItem64() const { return 0; }
    void SetItem64(int) {}
    int GetItem65() const { return 0; }
    void SetItem65(int) {}
    int GetItem66() const { return 0; }
    void SetItem66(int) {}
    int GetItem67() const { return 0; }
    void SetItem67(int) {}
    int GetItem68() const { return 0; }
    void SetItem68(int) {}
    int GetItem69() const { return 0; }
    void SetItem69(int) {}
    int GetItem70() const { return 0; }
    void SetItem70(int) {}
    int GetItem71() const { return 0; }
    void SetItem71(int) {}
    int GetItem72() const { return 0; }
    void SetItem72(int) {}
    int GetItem73() const { return 0; }
    void SetItem73(int) {}
    int GetItem74() const { return 0; }
    void SetItem74(int) {}
    int GetItem75() const { return 0; }
    void SetItem75(int) {}
    int GetItem76() const { return 0; }
    void SetItem76(int) {}
    int GetItem77() const { return 0; }
    void SetItem77(int) {}
    int GetItem78() const { return 0; }
    void SetItem78(int) {}
    int GetItem79() const { return 0; }
    void SetItem79(int) {}
    int GetItem80() const { return 0; }
    void SetItem80(int) {}
    int GetItem81() const { return 0; }
    void SetItem81(int) {}
    int GetItem82() const { return 0; }
    void SetItem82(int) {}
    int GetItem83() const { return 0; }
    void SetItem83(int) {}
    int GetItem84() const { return 0; }
    void SetItem84(int) {}
    int GetItem85() const { return 0; }
    void SetItem85(int) {}
    int GetItem86() const { return 0; }
    void SetItem86(int) {}
    int GetItem87() const { return 0; }
    void SetItem87(int) {}
    int GetItem88() const { return 0; }
    void SetItem88(int) {}
    int GetItem89() const { return 0; }
    void SetItem89(int) {}
    int GetItem90() const { return 0; }
    void SetItem90(int) {}
    int GetItem91() const { return 0; }
    void SetItem91(int) {}
    int GetItem92() const { return 0; }
    void SetItem92(int) {}
    int GetItem93() const { return 0; }
    void SetItem93(int) {}
    int GetItem94() const { return 0; }
    void SetItem94(int) {}
    int GetItem95() const { return 0; }
    void SetItem95(int) {}
    int GetItem96() const { return 0; }
    void SetItem96(int) {}
    int GetItem97() const { return 0; }
    void SetItem97(int) {}
    int GetItem98() const { return 0; }
    void SetItem98(int) {}
    int GetItem99() const { return 0; }
    void SetItem99(int) {}
#ifdef PROPERTIES_2X
    int GetItem100() const { return 0; }
    void SetItem100(int) {}
    int GetItem101() const { return 0; }
    void SetItem101(int) {}
    int GetItem102() const { return 0; }
    void SetItem102(int) {}
    int GetItem103() const { return 0; }
    void SetItem103(int) {}
    int GetItem104() const { return 0; }
    void SetItem104(int) {}
    int GetItem105() const { return 0; }
    void SetItem105(int) {}
    int GetItem106() const { return 0; }
    void SetItem106(int) {}
    int GetItem107() const { return 0; }
    void SetItem107(int) {}
    int GetItem108() const { return 0; }
    void SetItem108(int) {}
    int GetItem109() const { return 0; }
    void SetItem109(int) {}
    int GetItem110() const { return 0; }
    void SetItem110(int) {}
    int GetItem111() const { return 0; }
    void SetItem111(int) {}
    int GetItem112() const { return 0; }
    void SetItem112(int) {}
    int GetItem113() const { return 0; }
    void SetItem113(int) {}
    int GetItem114() const { return 0; }
    void SetItem114(int) {}
    int GetItem115() const { return 0; }
    void SetItem115(int) {}
    int GetItem116() const { return 0; }
    void SetItem116(int) {}
    int GetItem117() const { return 0; }
    void SetItem117(int) {}
    int GetItem118() const { return 0; }
    void SetItem118(int) {}
    int GetItem119() const { return 0; }
    void SetItem119(int) {}
    int GetItem120() const { return 0; }
    void SetItem120(int) {}
    int GetItem121() const { return 0; }
    void SetItem121(int) {}
    int GetItem122() const { return 0; }
    void SetItem122(int) {}
    int GetItem123() const { return 0; }
    void SetItem123(int) {}
    int GetItem124() const { return 0; }
    void SetItem124(int) {}
    int GetItem125() const { return 0; }
    void SetItem125(int) {}
    int GetItem126() const { return 0; }
    void SetItem126(int) {}
    int GetItem127() const { return 0; }
    void SetItem127(int) {}
    int GetItem128() const { return 0; }
    void SetItem128(int) {}
    int GetItem129() const { return 0; }
    void SetItem129(int) {}
    int GetItem130() const { return 0; }
    void SetItem130(int) {}
    int GetItem131() const { return 0; }
    void SetItem131(int) {}
    int GetItem132() const { return 0; }
    void SetItem132(int) {}
    int GetItem133() const { return 0; }
    void SetItem133(int) {}
    int GetItem134() const { return 0; }
    void SetItem134(int) {}
    int GetItem135() const { return 0; }
    void SetItem135(int) {}
    int GetItem136() const { return 0; }
    void SetItem136(int) {}
    int GetItem137() const { return 0; }
    void SetItem137(int) {}
    int GetItem138() const { return 0; }
    void SetItem138(int) {}
    int GetItem139() const { return 0; }
    void SetItem139(int) {}
    int GetItem140() const { return 0; }
    void SetItem140(int) {}
    int GetItem141() const { return 0; }
    void SetItem141(int) {}
    int GetItem142() const { return 0; }
    void SetItem142(int) {}
    int GetItem143() const { return 0; }
    void SetItem143(int) {}
    int GetItem144() const { return 0; }
    void SetItem144(int) {}
    int GetItem145() const { return 0; }
    void SetItem145(int) {}
    int GetItem146() const { return 0; }
    void SetItem146(int) {}
    int GetItem147() const { return 0; }
    void SetItem147(int) {}
    int GetItem148() const { return 0; }
    void SetItem148(int) {}
    int GetItem149() const { return 0; }
    void SetItem149(int) {}
    int GetItem150() const { return 0; }
    void SetItem150(int) {}
    int GetItem151() const { return 0; }
    void SetItem151(int) {}
    int GetItem152() const { return 0; }
    void SetItem152(int) {}
    int GetItem153() const { return 0; }
    void SetItem153(int) {}
    int GetItem154() const { return 0; }
    void SetItem154(int) {}
    int GetItem155() const { return 0; }
    void SetItem155(int) {}
    int GetItem156() const { return 0; }
    void SetItem156(int) {}
    int GetItem157() const { return 0; }
    void SetItem157(int) {}
    int GetItem158() const { return 0; }
    void SetItem158(int) {}
    int GetItem159() const { return 0; }
    void SetItem159(int) {}
    int GetItem160() const { return 0; }
    void SetItem160(int) {}
    int GetItem161() const { return 0; }
    void SetItem161(int) {}
    int GetItem162() const { return 0; }
    void SetItem162(int) {}
    int GetItem163() const { return 0; }
    void SetItem163(int) {}
    int GetItem164() const { return 0; }
    void SetItem164(int) {}
    int GetItem165() const { return 0; }
    void SetItem165(int) {}
    int GetItem166() const { return 0; }
    void SetItem166(int) {}
    int GetItem167() const { return 0; }
    void SetItem167(int) {}
    int GetItem168() const { return 0; }
    void SetItem168(int) {}
    int GetItem169() const { return 0; }
    void SetItem169(int) {}
    int GetItem170() const { return 0; }
    void SetItem170(int) {}
    int GetItem171() const { return 0; }
    void SetItem171(int) {}
    int GetItem172() const { return 0; }
    void SetItem172(int) {}
    int GetItem173() const { return 0; }
    void SetItem173(int) {}
    int GetItem174() const { return 0; }
    void SetItem174(int) {}
    int GetItem175() const { return 0; }
    void SetItem175(int) {}
    int GetItem176() const { return 0; }
    void SetItem176(int) {}
    int GetItem177() const { return 0; }
    void SetItem177(int) {}
    int GetItem178() const { return 0; }
    void SetItem178(int) {}
    int GetItem179() const { return 0; }
    void SetItem179(int) {}
    int GetItem180() const { return 0; }
    void SetItem180(int) {}
    int GetItem181() const { return 0; }
    void SetItem181(int) {}
    int GetItem182() const { return 0; }
    void SetItem182(int) {}
    int GetItem183() const { return 0; }
    void SetItem183(int) {}
    int GetItem184() const { return 0; }
    void SetItem184(int) {}
    int GetItem185() const { return 0; }
    void SetItem185(int) {}
    int GetItem186() const { return 0; }
    void SetItem186(int) {}
    int GetItem187() const { return 0; }
    void SetItem187(int) {}
    int GetItem188() const { return 0; }
    void SetItem188(int) {}
    int GetItem189() const { return 0; }
    void SetItem189(int) {}
    int GetItem190() const { return 0; }
    void SetItem190(int) {}
    int GetItem191() const { return 0; }
    void SetItem191(int) {}
    int GetItem192() const { return 0; }
    void SetItem192(int) {}
    int GetItem193() const { return 0; }
    void SetItem193(int) {}
    int GetItem194() const { return 0; }
    void SetItem194(int) {}
    int GetItem195() const { return 0; }
    void SetItem195(int) {}
    int GetItem196() const { return 0; }
    void SetItem196(int) {}
    int GetItem197() const { return 0; }
    void SetItem197(int) {}
    int GetItem198() const { return 0; }
    void SetItem198(int) {}
    int GetItem199() const { return 0; }
    void SetItem199(int) {}
#endif // PROPERTIES_2X
};

REFL_TYPE(LargePod)
    REFL_FUNC(GetItem0, property())
    REFL_FUNC(SetItem0, property())
    REFL_FUNC(GetItem1, property())
    REFL_FUNC(SetItem1, property())
    REFL_FUNC(GetItem2, property())
    REFL_FUNC(SetItem2, property())
    REFL_FUNC(GetItem3, property())
    REFL_FUNC(SetItem3, property())
    REFL_FUNC(GetItem4, property())
    REFL_FUNC(SetItem4, property())
    REFL_FUNC(GetItem5, property())
    REFL_FUNC(SetItem5, property())
    REFL_FUNC(GetItem6, property())
    REFL_FUNC(SetItem6, property())
    REFL_FUNC(GetItem7, property())
    REFL_FUNC(SetItem7, property())
    REFL_FUNC(GetItem8, property())
    REFL_FUNC(SetItem8, property())
    REFL_FUNC(GetItem9, property())
    REFL_FUNC(SetItem9, property())
    REFL_FUNC(GetItem10, property())
    REFL_FUNC(SetItem10, property())
    REFL_FUNC(GetItem11, property())
    REFL_FUNC(SetItem11, property())
    REFL_FUNC(GetItem12, property())
    REFL_FUNC(SetItem12, property())
    REFL_FUNC(GetItem13, property())
    REFL_FUNC(SetItem13, property())
    REFL_FUNC(GetItem14, property())
    REFL_FUNC(SetItem14, property())
    REFL_FUNC(GetItem15, property())
    REFL_FUNC(SetItem15, property())
    REFL_FUNC(GetItem16, property())
    REFL_FUNC(SetItem16, property())
    REFL_FUNC(GetItem17, property())
    REFL_FUNC(SetItem17, property())
    REFL_FUNC(GetItem18, property())
    REFL_FUNC(SetItem18, property())
    REFL_FUNC(GetItem19, property())
    REFL_FUNC(SetItem19, property())
    REFL_FUNC(GetItem20, property())
    REFL_FUNC(SetItem20, property())
    REFL_FUNC(GetItem21, property())
    REFL_FUNC(SetItem21, property())
    REFL_FUNC(GetItem22, property())
    REFL_FUNC(SetItem22, property())
    REFL_FUNC(GetItem23, property())
    REFL_FUNC(SetItem23, property())
    REFL_FUNC(GetItem24, property())
    REFL_FUNC(SetItem24, property())
    REFL_FUNC(GetItem25, property())
    REFL_FUNC(SetItem25, property())
    REFL_FUNC(GetItem26, property())
    REFL_FUNC(SetItem26, property())
    REFL_FUNC(GetItem27, property())
    REFL_FUNC(SetItem27, property())
    REFL_FUNC(GetItem28, property())
    REFL_FUNC(SetItem28, property())
    REFL_FUNC(GetItem29, property())
    REFL_FUNC(SetItem29, property())
    REFL_FUNC(GetItem30, property())
    REFL_FUNC(SetItem30, property())
    REFL_FUNC(GetItem31, property())
    REFL_FUNC(SetItem31, property())
    REFL_FUNC(GetItem32, property())
    REFL_FUNC(SetItem32, property())
    REFL_FUNC(GetItem33, property())
    REFL_FUNC(SetItem33, property())
    REFL_FUNC(GetItem34, property())
    REFL_FUNC(SetItem34, property())
    REFL_FUNC(GetItem35, property())
    REFL_FUNC(SetItem35, property())
    REFL_FUNC(GetItem36, property())
    REFL_FUNC(SetItem36, property())
    REFL_FUNC(GetItem37, property())
    REFL_FUNC(SetItem37, property())
    REFL_FUNC(GetItem38, property())
    REFL_FUNC(SetItem38, property())
    REFL_FUNC(GetItem39, property())
    REFL_FUNC(SetItem39, property())
    REFL_FUNC(GetItem40, property())
    REFL_FUNC(SetItem40, property())
    REFL_FUNC(GetItem41, property())
    REFL_FUNC(SetItem41, property())
    REFL_FUNC(GetItem42, property())
    REFL_FUNC(SetItem42, property())
    REFL_FUNC(GetItem43, property())
    REFL_FUNC(SetItem43, property())
    REFL_FUNC(GetItem44, property())
    REFL_FUNC(SetItem44, property())
    REFL_FUNC(GetItem45, property())
    REFL_FUNC(SetItem45, property())
    REFL_FUNC(GetItem46, property())
    REFL_FUNC(SetItem46, property())
    REFL_FUNC(GetItem47, property())
    REFL_FUNC(SetItem47, property())
    REFL_FUNC(GetItem48, property())
    REFL_FUNC(SetItem48, property())
    REFL_FUNC(GetItem49, property())
    REFL_FUNC(SetItem49, property())
    REFL_FUNC(GetItem50, property())
    REFL_FUNC(SetItem50, property())
    REFL_FUNC(GetItem51, property())
    REFL_FUNC(SetItem51, property())
    REFL_FUNC(GetItem52, property())
    REFL_FUNC(SetItem52, property())
    REFL_FUNC(GetItem53, property())
    REFL_FUNC(SetItem53, property())
    REFL_FUNC(GetItem54, property())
    REFL_FUNC(SetItem54, property())
    REFL_FUNC(GetItem55, property())
    REFL_FUNC(SetItem55, property())
    REFL_FUNC(GetItem56, property())
    REFL_FUNC(SetItem56, property())
    REFL_FUNC(GetItem57, property())
    REFL_FUNC(SetItem57, property())
    REFL_FUNC(GetItem58, property())
    REFL_FUNC(SetItem58, property())
    REFL_FUNC(GetItem59, property())
    REFL_FUNC(SetItem59, property())
    REFL_FUNC(GetItem60, property())
    REFL_FUNC(SetItem60, property())
    REFL_FUNC(GetItem61, property())
    REFL_FUNC(SetItem61, property())
    REFL_FUNC(GetItem62, property())
    REFL_FUNC(SetItem62, property())
    REFL_FUNC(GetItem63, property())
    REFL_FUNC(SetItem63, property())
    REFL_FUNC(GetItem64, property())
    REFL_FUNC(SetItem64, property())
    REFL_FUNC(GetItem65, property())
    REFL_FUNC(SetItem65, property())
    REFL_FUNC(GetItem66, property())
    REFL_FUNC(SetItem66, property())
    REFL_FUNC(GetItem67, property())
    REFL_FUNC(SetItem67, property())
    REFL_FUNC(GetItem68, property())
    REFL_FUNC(SetItem68, property())
    REFL_FUNC(GetItem69, property())
    REFL_FUNC(SetItem69, property())
    REFL_FUNC(GetItem70, property())
    REFL_FUNC(SetItem70, property())
    REFL_FUNC(GetItem71, property())
    REFL_FUNC(SetItem71, property())
    REFL_FUNC(GetItem72, property())
    REFL_FUNC(SetItem72, property())
    REFL_FUNC(GetItem73, property())
    REFL_FUNC(SetItem73, property())
    REFL_FUNC(GetItem74, property())
    REFL_FUNC(SetItem74, property())
    REFL_FUNC(GetItem75, property())
    REFL_FUNC(SetItem75, property())
    REFL_FUNC(GetItem76, property())
    REFL_FUNC(SetItem76, property())
    REFL_FUNC(GetItem77, property())
    REFL_FUNC(SetItem77, property())
    REFL_FUNC(GetItem78, property())
    REFL_FUNC(SetItem78, property())
    REFL_FUNC(GetItem79, property())
    REFL_FUNC(SetItem79, property())
    REFL_FUNC(GetItem80, property())
    REFL_FUNC(SetItem80, property())
    REFL_FUNC(GetItem81, property())
    REFL_FUNC(SetItem81, property())
    REFL_FUNC(GetItem82, property())
    REFL_FUNC(SetItem82, property())
    REFL_FUNC(GetItem83, property())
    REFL_FUNC(SetItem83, property())
    REFL_FUNC(GetItem84, property())
    REFL_FUNC(SetItem84, property())
    REFL_FUNC(GetItem85, property())
    REFL_FUNC(SetItem85, property())
    REFL_FUNC(GetItem86, property())
    REFL_FUNC(SetItem86, property())
    REFL_FUNC(GetItem87, property())
    REFL_FUNC(SetItem87, property())
    REFL_FUNC(GetItem88, property())
    REFL_FUNC(SetItem88, property())
    REFL_FUNC(GetItem89, property())
    REFL_FUNC(SetItem89, property())
    REFL_FUNC(GetItem90, property())
    REFL_FUNC(SetItem90, property())
    REFL_FUNC(GetItem91, property())
    REFL_FUNC(SetItem91, property())
    REFL_FUNC(GetItem92, property())
    REFL_FUNC(SetItem92, property())
    REFL_FUNC(GetItem93, property())
    REFL_FUNC(SetItem93, property())
    REFL_FUNC(GetItem94, property())
    REFL_FUNC(SetItem94, property())
    REFL_FUNC(GetItem95, property())
    REFL_FUNC(SetItem95, property())
    REFL_FUNC(GetItem96, property())
    REFL_FUNC(SetItem96, property())
    REFL_FUNC(GetItem97, property())
    REFL_FUNC(SetItem97, property())
    REFL_FUNC(GetItem98, property())
    REFL_FUNC(SetItem98, property())
    REFL_FUNC(GetItem99, property())
    REFL_FUNC(SetItem99, property())
#ifdef PROPERTIES_2X
    REFL_FUNC(GetItem100, property())
    REFL_FUNC(SetItem100, property())
    REFL_FUNC(GetItem101, property())
    REFL_FUNC(SetItem101, property())
    REFL_FUNC(GetItem102, property())
    REFL_FUNC(SetItem102, property())
    REFL_FUNC(GetItem103, property())
    REFL_FUNC(SetItem103, property())
    REFL_FUNC(GetItem104, property())
    REFL_FUNC(SetItem104, property())
    REFL_FUNC(GetItem105, property())
    REFL_FUNC(SetItem105, property())
    REFL_FUNC(GetItem106, property())
    REFL_FUNC(SetItem106, property())
    REFL_FUNC(GetItem107, property())
    REFL_FUNC(SetItem107, property())
    REFL_FUNC(GetItem108, property())
    REFL_FUNC(SetItem108, property())
    REFL_FUNC(GetItem109, property())
    REFL_FUNC(SetItem109, property())
    REFL_FUNC(GetItem110, property())
    REFL_FUNC(SetItem110, property())
    REFL_FUNC(GetItem111, property())
    REFL_FUNC(SetItem111, property())
    REFL_FUNC(GetItem112, property())
    REFL_FUNC(SetItem112, property())
    REFL_FUNC(GetItem113, property())
    REFL_FUNC(SetItem113, property())
    REFL_FUNC(GetItem114, property())
    REFL_FUNC(SetItem114, property())
    REFL_FUNC(GetItem115, property())
    REFL_FUNC(SetItem115, property())
    REFL_FUNC(GetItem116, property())
    REFL_FUNC(SetItem116, property())
    REFL_FUNC(GetItem117, property())
    REFL_FUNC(SetItem117, property())
    REFL_FUNC(GetItem118, property())
    REFL_FUNC(SetItem118, property())
    REFL_FUNC(GetItem119, property())
    REFL_FUNC(SetItem119, property())
    REFL_FUNC(GetItem120, property())
    REFL_FUNC(SetItem120, property())
    REFL_FUNC(GetItem121, property())
    REFL_FUNC(SetItem121, property())
    REFL_FUNC(GetItem122, property())
    REFL_FUNC(SetItem122, property())
    REFL_FUNC(GetItem123, property())
    REFL_FUNC(SetItem123, property())
    REFL_FUNC(GetItem124, property())
    REFL_FUNC(SetItem124, property())
    REFL_FUNC(GetItem125, property())
    REFL_FUNC(SetItem125, property())
    REFL_FUNC(GetItem126, property())
    REFL_FUNC(SetItem126, property())
    REFL_FUNC(GetItem127, property())
    REFL_FUNC(SetItem127, property())
    REFL_FUNC(GetItem128, property())
    REFL_FUNC(SetItem128, property())
    REFL_FUNC(GetItem129, property())
    REFL_FUNC(SetItem129, property())
    REFL_FUNC(GetItem130, property())
    REFL_FUNC(SetItem130, property())
    REFL_FUNC(GetItem131, property())
    REFL_FUNC(SetItem131, property())
    REFL_FUNC(GetItem132, property())
    REFL_FUNC(SetItem132, property())
    REFL_FUNC(GetItem133, property())
    REFL_FUNC(SetItem133, property())
    REFL_FUNC(GetItem134, property())
    REFL_FUNC(SetItem134, property())
    REFL_FUNC(GetItem135, property())
    REFL_FUNC(SetItem135, property())
    REFL_FUNC(GetItem136, property())
    REFL_FUNC(SetItem136, property())
    REFL_FUNC(GetItem137, property())
    REFL_FUNC(SetItem137, property())
    REFL_FUNC(GetItem138, property())
    REFL_FUNC(SetItem138, property())
    REFL_FUNC(GetItem139, property())
    REFL_FUNC(SetItem139, property())
    REFL_FUNC(GetItem140, property())
    REFL_FUNC(SetItem140, property())
    REFL_FUNC(GetItem141, property())
    REFL_FUNC(SetItem141, property())
    REFL_FUNC(GetItem142, property())
    REFL_FUNC(SetItem142, property())
    REFL_FUNC(GetItem143, property())
    REFL_FUNC(SetItem143, property())
    REFL_FUNC(GetItem144, property())
    REFL_FUNC(SetItem144, property())
    REFL_FUNC(GetItem145, property())
    REFL_FUNC(SetItem145, property())
    REFL_FUNC(GetItem146, property())
    REFL_FUNC(SetItem146, property())
    REFL_FUNC(GetItem147, property())
    REFL_FUNC(SetItem147, property())
    REFL_FUNC(GetItem148, property())
    REFL_FUNC(SetItem148, property())
    REFL_FUNC(GetItem149, property())
    REFL_FUNC(SetItem149, property())
    REFL_FUNC(GetItem150, property())
    REFL_FUNC(SetItem150, property())
    REFL_FUNC(GetItem151, property())
    REFL_FUNC(SetItem151, property())
    REFL_FUNC(GetItem152, property())
    REFL_FUNC(SetItem152, property())
    REFL_FUNC(GetItem153, property())
    REFL_FUNC(SetItem153, property())
    REFL_FUNC(GetItem154, property())
    REFL_FUNC(SetItem154, property())
    REFL_FUNC(GetItem155, property())
    REFL_FUNC(SetItem155, property())
    REFL_FUNC(GetItem156, property())
    REFL_FUNC(SetItem156, property())
    REFL_FUNC(GetItem157, property())
    REFL_FUNC(SetItem157, property())
    REFL_FUNC(GetItem158, property())
    REFL_FUNC(SetItem158, property())
    REFL_FUNC(GetItem159, property())
    REFL_FUNC(SetItem159, property())
    REFL_FUNC(GetItem160, property())
    REFL_FUNC(SetItem160, property())
    REFL_FUNC(GetItem161, property())
    REFL_FUNC(SetItem161, property())
    REFL_FUNC(GetItem162, property())
    REFL_FUNC(SetItem162, property())
    REFL_FUNC(GetItem163, property())
    REFL_FUNC(SetItem163, property())
    REFL_FUNC(GetItem164, property())
    REFL_FUNC(SetItem164, property())
    REFL_FUNC(GetItem165, property())
    REFL_FUNC(SetItem165, property())
    REFL_FUNC(GetItem166, property())
    REFL_FUNC(SetItem166, property())
    REFL_FUNC(GetItem167, property())
    REFL_FUNC(SetItem167, property())
    REFL_FUNC(GetItem168, property())
    REFL_FUNC(SetItem168, property())
    REFL_FUNC(GetItem169, property())
    REFL_FUNC(SetItem169, property())
    REFL_FUNC(GetItem170, property())
    REFL_FUNC(SetItem170, property())
    REFL_FUNC(GetItem171, property())
    REFL_FUNC(SetItem171, property())
    REFL_FUNC(GetItem172, property())
    REFL_FUNC(SetItem172, property())
    REFL_FUNC(GetItem173, property())
    REFL_FUNC(SetItem173, property())
    REFL_FUNC(GetItem174, property())
    REFL_FUNC(SetItem174, property())
    REFL_FUNC(GetItem175, property())
    REFL_FUNC(SetItem175, property())
    REFL_FUNC(GetItem176, property())
    REFL_FUNC(SetItem176, property())
    REFL_FUNC(GetItem177, property())
    REFL_FUNC(SetItem177, property())
    REFL_FUNC(GetItem178, property())
    REFL_FUNC(SetItem178, property())
    REFL_FUNC(GetItem179, property())
    REFL_FUNC(SetItem179, property())
    REFL_FUNC(GetItem180, property())
    REFL_FUNC(SetItem180, property())
    REFL_FUNC(GetItem181, property())
    REFL_FUNC(SetItem181, property())
    REFL_FUNC(GetItem182, property())
    REFL_FUNC(SetItem182, property())
    REFL_FUNC(GetItem183, property())
    REFL_FUNC(SetItem183, property())
    REFL_FUNC(GetItem184, property())
    REFL_FUNC(SetItem184, property())
    REFL_FUNC(GetItem185, property())
    REFL_FUNC(SetItem185, property())
    REFL_FUNC(GetItem186, property())
    REFL_FUNC(SetItem186, property())
    REFL_FUNC(GetItem187, property())
    REFL_FUNC(SetItem187, property())
    REFL_FUNC(GetItem188, property())
    REFL_FUNC(SetItem188, property())
    REFL_FUNC(GetItem189, property())
    REFL_FUNC(SetItem189, property())
    REFL_FUNC(GetItem190, property())
    REFL_FUNC(SetItem190, property())
    REFL_FUNC(GetItem191, property())
    REFL_FUNC(SetItem191, property())
    REFL_FUNC(GetItem192, property())
    REFL_FUNC(SetItem192, property())
    REFL_FUNC(GetItem193, property())
    REFL_FUNC(SetItem193, property())
    REFL_FUNC(GetItem194, property())
    REFL_FUNC(SetItem194, property())
    REFL_FUNC(GetItem195, property())
    REFL_FUNC(SetItem195, property())
    REFL_FUNC(GetItem196, property())
    REFL_FUNC(SetItem196, property())
    REFL_FUNC(GetItem197, property())
    REFL_FUNC(SetItem197, property())
    REFL_FUNC(GetItem198, property())
    REFL_FUNC(SetItem198, property())
    REFL_FUNC(GetItem199, property())
    REFL_FUNC(SetItem199, property())
#endif // PROPERTIES_2X
REFL_END

int main()
{
    ProcessClass<LargePod>();
}
