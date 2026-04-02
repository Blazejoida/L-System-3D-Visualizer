#include "LSystem.h"

// =============================================================
//  ABoP turtle (Prusinkiewicz & Lindenmayer):
//  DRAW:  F G           (all other letters = grammar nodes, ignored)
//  MOVE:  f             (no segment)
//  YAW:   + -           (RU +/-delta)
//  PITCH: & ^           (RL +/-delta)
//  ROLL:  \ /           (RH +/-delta, official ABoP symbols)
//  UTURN: |             (RU 180)
//  STACK: [ ]
// =============================================================

static BranchColor rgb(float r, float g, float b) { return{ r,g,b }; }

static LSystem make(
    const char* name, const char* desc, const char* axiom,
    std::initializer_list<std::pair<char, const char*>> rules,
    float angle, float scale,
    BranchColor trunk, BranchColor branch, BranchColor leaf)
{
    LSystem ls;
    ls.name = name; ls.description = desc; ls.axiom = axiom;
    for (auto& r : rules)ls.rules[r.first] = r.second;
    ls.angle = angle; ls.stepLen = 1.f; ls.stepScale = scale;
    ls.colTrunk = trunk; ls.colBranch = branch; ls.colLeaf = leaf;
    return ls;
}

std::vector<LSystem> buildPresets()
{
    std::vector<LSystem> p;

    // ==========================================================
    //  ABoP CANONICAL PLANTS  ch.1, fig.1.24 
    //  Only F draws. X Y = non-drawing grammar nodes.
    //  stepScale=1.0 — crown density from F->FF.
    // ==========================================================

    // fig.1.24a  delta=25  edge rewriting
    p.push_back(make("ABoP 1.24a",
        "fig.1.24a: F[+F]F[-F]F  delta=25  iter 5",
        "F", { {'F',"F[+F]F[-F]F"} },
        25.f, 1.f,
        rgb(.40f, .22f, .06f), rgb(.20f, .52f, .09f), rgb(.16f, .70f, .12f)));

    // fig.1.24b  delta=20  edge rewriting
    p.push_back(make("ABoP 1.24b",
        "fig.1.24b: F[+F]F[-F][F]  delta=20  iter 5",
        "F", { {'F',"F[+F]F[-F][F]"} },
        20.f, 1.f,
        rgb(.40f, .22f, .06f), rgb(.20f, .52f, .09f), rgb(.16f, .70f, .12f)));

    // fig.1.24c  delta=22.5  THE Prusinkiewicz tree
    p.push_back(make("ABoP 1.24c (classic)",
        "fig.1.24c: FF-[-F+F+F]+[+F-F-F]  delta=22.5  iter 5",
        "F", { {'F',"FF-[-F+F+F]+[+F-F-F]"} },
        22.5f, 1.f,
        rgb(.40f, .22f, .06f), rgb(.20f, .52f, .09f), rgb(.16f, .70f, .12f)));

    // fig.1.24d  delta=20  node rewriting  X=growth tip
    p.push_back(make("ABoP 1.24d",
        "fig.1.24d: X=F[+X]F[-X]+X  F=FF  delta=20  iter 10",
        "X", { {'X',"F[+X]F[-X]+X"},{'F',"FF"} },
        20.f, 1.f,
        rgb(.38f, .20f, .05f), rgb(.18f, .52f, .08f), rgb(.14f, .72f, .10f)));

    // fig.1.24e  delta=25.7  sympodial
    p.push_back(make("ABoP 1.24e (sympodial)",
        "fig.1.24e: X=F[+X][-X]FX  F=FF  delta=25.7  iter 10",
        "X", { {'X',"F[+X][-X]FX"},{'F',"FF"} },
        25.7f, 1.f,
        rgb(.38f, .20f, .05f), rgb(.18f, .52f, .08f), rgb(.14f, .72f, .10f)));

    // fig.1.24f  delta=22.5  asymmetric sympodial
    p.push_back(make("ABoP 1.24f (asymmetric)",
        "fig.1.24f: X=F-[[X]+X]+F[+FX]-X  F=FF  delta=22.5  iter 9",
        "X", { {'X',"F-[[X]+X]+F[+FX]-X"},{'F',"FF"} },
        22.5f, 1.f,
        rgb(.38f, .20f, .05f), rgb(.18f, .52f, .08f), rgb(.14f, .72f, .10f)));

    // Barnsley Fern 
    p.push_back(make("Barnsley Fern",
        "X=F+[[X]-X]-F[-FX]+X  F=FF  delta=25  iter 9",
        "X", { {'X',"F+[[X]-X]-F[-FX]+X"},{'F',"FF"} },
        25.f, 1.f,
        rgb(.24f, .14f, .04f), rgb(.12f, .52f, .10f), rgb(.16f, .74f, .12f)));

    // ABoP bush
    p.push_back(make("ABoP Bush",
        "Y=YFX[+Y][-Y]  X=X[-FFF][+FFF]FX  delta=25.7  iter 9",
        "Y", { {'Y',"YFX[+Y][-Y]"},{'X',"X[-FFF][+FFF]FX"} },
        25.7f, 1.f,
        rgb(.40f, .24f, .08f), rgb(.20f, .56f, .09f), rgb(.16f, .74f, .11f)));

    // ==========================================================
    //  3D TREES 
    // ==========================================================

    // 3D Whorl Tree:  every internode has 6 lateral branches.
    // [&Y] = push, pitch branch 60 deg down, recurse Y, pop.
    // / between branches = roll trunk 60 deg (6 x 60 = 360 full circle).
    p.push_back(make("3D Whorl Tree",
        "6 branches/whorl, / 60-deg roll between each. iter 15",
        "X",
        { {'X',"FF[&Y]/[&Y]/[&Y]/[&Y]/[&Y]/[&Y]X"},
         {'Y',"F[+Y][-Y]"} },
        60.f, 0.8f,
        rgb(.38f, .20f, .05f), rgb(.20f, .52f, .08f), rgb(.16f, .72f, .12f)));

    // 3D Oak:  4 branches combining pitch down (&) and pitch up (^)
    // with yaw left (+) and right (-) plus / roll between nodes.
    p.push_back(make("3D Oak",
        "4 branches: [&+X][&-X][^+X][^-X] + / 90 roll. iter 8",
        "X",
        { {'X',"F[&+X][&-X][^+X][^-X]/X"},{'F',"FF"} },
        26.f, 0.9f,
        rgb(.40f, .22f, .05f), rgb(.22f, .52f, .08f), rgb(.18f, .74f, .12f)));

    // 3D Ternary:  3 branches in 3 distinct spatial directions.
    p.push_back(make("3D Ternary",
        "3 branches: [&X][^X][&+X] + / roll. iter 8",
        "X",
        { {'X',"F[&X][^X][&+X]/X"},{'F',"FF"} },
        28.f, 0.9f,
        rgb(.40f, .22f, .06f), rgb(.22f, .54f, .09f), rgb(.18f, .72f, .12f)));

    // 3D Sympodial:  fork into two pitched branches + roll.
    p.push_back(make("3D Sympodial",
        "Fork [&+X][&-X] + / roll each level. iter 11",
        "X",
        { {'X',"F[&+X][&-X]/X"},{'F',"FF"} },
        26.f, 1.f,
        rgb(.38f, .20f, .05f), rgb(.18f, .52f, .08f), rgb(.14f, .72f, .10f)));

    // 3D Spruce:  conical monopodial, 6-branch whorls.
    p.push_back(make("3D Spruce",
        "Conical, 6 whorl branches per node. iter 13",
        "X",
        { {'X',"FF[&Y]/[&Y]/[&Y]/[&Y]/[&Y]/[&Y]X"},
         {'Y',"F[+Y][-Y]F"} },
        22.f, 0.78f,
        rgb(.28f, .16f, .04f), rgb(.10f, .40f, .10f), rgb(.14f, .62f, .14f)));

    // Sakura 3D: 
    p.push_back(make("Sakura 3D",
        "3 branches/node, iter 8",
        "X",
        { {'X',"F[&X]\\\\[&+X]\\\\[&-X]FX"},{'F',"FF"} },
        28.f, 1.f,
        rgb(.30f, .13f, .10f), rgb(.56f, .18f, .22f), rgb(.98f, .65f, .78f)));

    // Weeping Willow:  branches pitched steeply (&&) and droop.
    p.push_back(make("Weeping Willow 3D",
        "Branches pitch && strongly, droop each level. iter 10",
        "X",
        { {'X',"FF[&&+Y][&&-Y]/[&&Y]X"},
         {'Y',"F&[+Y][-Y]F&"} },
        20.f, 0.9f,
        rgb(.42f, .26f, .06f), rgb(.28f, .62f, .06f), rgb(.34f, .78f, .10f)));

    // Bonsai 3D:  asymmetric, \ roll rotates branching plane each level.
    p.push_back(make("Bonsai 3D",
        "Asymmetric, \\\\ roll rotates plane each level. iter 10",
        "X",
        { {'X',"F[&+X]/[-&X][&&X]"},{'F',"FF"} },
        32.f, 1.f,
        rgb(.40f, .24f, .08f), rgb(.26f, .50f, .10f), rgb(.22f, .70f, .14f)));

    // ==========================================================
    //  2D FRACTALS 
    //  All verified to produce correct 2D shapes.
    // ==========================================================

    // Koch Snowflake 
    p.push_back(make("Koch Snowflake",
        "F=F+F--F+F  axiom=F++F++F   iter 3-5",
        "F++F++F", { {'F',"F+F--F+F"} },
        60.f, 1.f,
        rgb(.70f, .86f, 1.f), rgb(.80f, .92f, 1.f), rgb(1.f, 1.f, 1.f)));

    // Quadratic Koch Island
    p.push_back(make("Koch Island (quad)",
        "delta=90  iter 2-3",
        "F-F-F-F", { {'F',"F-F+F+FF-F-F+F"} },
        90.f, 1.f,
        rgb(.80f, .60f, .10f), rgb(.90f, .75f, .20f), rgb(1.f, .90f, .40f)));

    // Dragon Curve 
    // Two edge types as node-rewrite: X=Fl-type, Y=Fr-type
    p.push_back(make("Heighway Dragon",
        "X=X+YF+  Y=-FX-Y   iter 10-14",
        "FX", { {'X',"X+YF+"},{'Y',"-FX-Y"} },
        90.f, 1.f,
        rgb(.80f, .15f, .10f), rgb(.90f, .40f, .10f), rgb(1.f, .70f, .20f)));

    // Hilbert 2D 
    p.push_back(make("Hilbert 2D",
        "L=+RF-LFL-FR+  R=-LF+RFR+FL-  iter 3-7",
        "L",
        { {'L',"+RF-LFL-FR+"},{'R',"-LF+RFR+FL-"} },
        90.f, 1.f,
        rgb(.80f, .30f, .80f), rgb(.90f, .55f, .90f), rgb(1.f, .80f, 1.f)));

    // Levy C Curve
    p.push_back(make("Levy C Curve",
        "F=+F--F+  delta=45  iter 8-12",
        "F", { {'F',"+F--F+"} },
        45.f, 1.f,
        rgb(.90f, .40f, .20f), rgb(1.f, .60f, .30f), rgb(1.f, .85f, .50f)));

    // ==========================================================
    //  3D FRACTALS  (numerically verified via bbox test)
    // ==========================================================

    // Hilbert 3D
    // Four rules A B C D. Only F draws.
    p.push_back(make("Hilbert 3D (ABCD)",
        "4-rule cubic space-fill.  iter 3-4",
        "A",
        { {'A',"B-F+CFC+F-D&F^D-F+&&CFC+F+B//"},
         {'B',"A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//"},
         {'C',"|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//"},
         {'D',"|CFB-F+B|FA&F^A&&FB-F+B|FC//"} },
        90.f, 1.f,
        rgb(.60f, .20f, .80f), rgb(.75f, .40f, .90f), rgb(.95f, .70f, 1.f)));

    // Menger 3D
    // Menger sponge skeleton extended to 3D
    p.push_back(make("Menger 3D",
        "Menger sponge skeleton  iter 4",
        "F+F+F+F",
        { {'F',"F+F-F-F+F[&F+F-F-F+F][^F+F-F-F+F]"} },
        90.f, 1.f,
        rgb(.90f, .60f, .20f), rgb(.70f, .40f, .10f), rgb(1.f, .80f, .40f)));

    // Crystal Dendrite 3D
    // X branches in all 6 cubic axis directions simultaneously.
    p.push_back(make("Crystal Dendrite 3D",
        "6-axis dendrite: [+X][-X][&X][^X][/X][\\\\X]  iter 7-8",
        "X",
        { {'X',"F[+X][-X][&X][^X][/X][\\X]"} },
        90.f, 1.f,
        rgb(.70f, .85f, 1.f), rgb(.80f, .92f, 1.f), rgb(1.f, 1.f, 1.f)));

    // Dandelion 3D
// X branches in all 6 cubic axis directions simultaneously.
    p.push_back(make("Dandelion 3D",
        "Common dandelion, 6-axis dendrite.  iter 7-8",
        "X",
        { {'X',"F[+X][-X][&X][^X][/X][\\X]"} },
        23.f, 1.f,
        rgb(.40f, .22f, .06f), rgb(.20f, .52f, .09f), rgb(1.f, 1.f, 1.f)));

    return p;
}

std::string expandLSystem(const LSystem& ls, int iters)
{
    std::string cur = ls.axiom;
    for (int i = 0; i < iters; ++i) {
        std::string nx; nx.reserve(cur.size() * 5);
        for (char c : cur) {
            auto it = ls.rules.find(c);
            if (it != ls.rules.end())nx += it->second; else nx += c;
        }
        cur = std::move(nx);
        if (cur.size() > 6000000u)break;
    }
    return cur;
}