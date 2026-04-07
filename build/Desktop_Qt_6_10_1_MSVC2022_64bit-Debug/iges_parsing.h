#pragma once
#include "nurbs.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <fstream>
#include <iomanip>

inline std::string readFileToString(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline bool parsePSectionEntity(const std::string& text, int startSeq,
                                     std::vector<std::string>& outTokens)
{
    std::istringstream stream(text);
    std::string line;

    bool collecting = false;
    std::string collected;
    collected.reserve(16384);

    while (std::getline(stream, line))
    {
        if (line.size() < 80) continue;
        if (line[72] != 'P') continue;

        int seq = 0;
        try {
            seq = std::stoi(line.substr(73, 7));
        } catch (...) {
            continue;
        }

        if (seq == startSeq)
        {
            collecting = true;
            collected.clear();
        }

        if (!collecting) continue;

        collected += line.substr(0, 64);

        auto semi = collected.find(';');
        if (semi != std::string::npos)
        {
            collected = collected.substr(0, semi);
            collecting = false;
            break;
        }
    }

    if (collected.empty()) return false;

    collected.erase(std::remove(collected.begin(), collected.end(), ' '), collected.end());

    for (size_t i = 1; i + 1 < collected.size(); ++i)
    {
        if (collected[i] == 'D' &&
            (std::isdigit(collected[i - 1]) || collected[i - 1] == '.') &&
            (collected[i + 1] == '+' || collected[i + 1] == '-' || std::isdigit(collected[i + 1])))
        {
            collected[i] = 'E';
        }
    }

    std::vector<std::string> tokens;
    tokens.reserve(8192);
    {
        std::stringstream ss(collected);
        std::string tok;
        while (std::getline(ss, tok, ','))
        {
            if (!tok.empty())
                tokens.push_back(tok);
        }
    }

    if (tokens.size() < 12) return false;

    outTokens = std::move(tokens);
    return true;
}

inline bool buildSurfaceFromTokens(const std::vector<std::string>& tokens,
                                        NurbsSurface& surfOut)
{
    if (tokens.empty() || tokens[0] != "128") return false;

    int K1 = 0, K2 = 0, M1 = 0, M2 = 0;
    try {
        K1 = std::stoi(tokens[1]);
        K2 = std::stoi(tokens[2]);
        M1 = std::stoi(tokens[3]);
        M2 = std::stoi(tokens[4]);
    } catch (...) {
        return false;
    }

    const int nu = K1 + 1;
    const int nv = K2 + 1;
    const long long n = 1LL * nu * nv;

    if (nu <= 0 || nv <= 0) return false;
    if (nu > 10000 || nv > 10000) return false;
    if (n > 5'000'000) return false;

    const int uKnotCount = K1 + M1 + 2;
    const int vKnotCount = K2 + M2 + 2;

    for (int uStart : {10, 9})
    {
        const int vStart  = uStart + uKnotCount;
        const int wStart  = vStart + vKnotCount;
        const int cpStart = wStart + (int)n;
        const int cpEnd   = cpStart + 3 * (int)n;

        if ((int)tokens.size() < cpEnd) continue;

        NurbsSurface s;
        s.K1 = K1; s.K2 = K2;
        s.M1 = M1; s.M2 = M2;

        s.U.reserve(uKnotCount);
        s.V.reserve(vKnotCount);
        s.W.reserve((int)n);
        s.P.reserve((int)n);

        try {
            for (int i = 0; i < uKnotCount; ++i)
                s.U.push_back(std::stod(tokens[uStart + i]));
            for (int i = 0; i < vKnotCount; ++i)
                s.V.push_back(std::stod(tokens[vStart + i]));
            for (int i = 0; i < (int)n; ++i)
                s.W.push_back(std::stod(tokens[wStart + i]));

            for (int i = 0; i < (int)n; ++i)
            {
                const double x = std::stod(tokens[cpStart + 3 * i + 0]);
                const double y = std::stod(tokens[cpStart + 3 * i + 1]);
                const double z = std::stod(tokens[cpStart + 3 * i + 2]);
                s.P.push_back(QVector3D((float)x, (float)y, (float)z));
            }
        } catch (...) {
            continue;
        }

        if ((int)s.U.size() != uKnotCount || (int)s.V.size() != vKnotCount) continue;
        if ((int)s.W.size() != (int)n || (int)s.P.size() != (int)n) continue;

        const int p = s.M1;
        const int q = s.M2;
        const int mU = (int)s.U.size() - 1;
        const int mV = (int)s.V.size() - 1;
        if (p < 0 || q < 0) continue;
        if (mU - p < 0 || mV - q < 0) continue;
        if (p >= (int)s.U.size() || q >= (int)s.V.size()) continue;

        surfOut = std::move(s);
        return true;
    }

    return false;
}

inline std::vector<int> scanDSectionForEntity128(const std::string& text)
{
    std::istringstream stream(text);
    std::string line;
    int dLineCounter = 0;
    std::vector<int> pointers;

    while (std::getline(stream, line))
    {
        if (line.size() < 73 || line[72] != 'D') continue;
        if ((dLineCounter % 2) == 0 && line.size() >= 16)
        {
            int type = 0, ptr = 0;
            try {
                type = std::stoi(line.substr(0, 8));
                ptr  = std::stoi(line.substr(8, 8));
            } catch (...) {}
            if (type == 128) pointers.push_back(ptr);
        }
        dLineCounter++;
    }
    return pointers;
}

// Flip V direction of a parsed NurbsSurface (reverses normal)
inline void flipSurfaceV(NurbsSurface& s)
{
    int nu = s.K1 + 1;
    int nv = s.K2 + 1;

    // Reverse V knot vector
    double vMin = s.V.front();
    double vMax = s.V.back();
    std::vector<double> newV(s.V.size());
    for (int i = 0; i < (int)s.V.size(); ++i)
        newV[i] = vMax + vMin - s.V[s.V.size() - 1 - i];
    s.V = newV;

    // Reverse control points and weights along V
    // Storage is column-major: index = j * nu + i
    // Each "column" j has nu points. We reverse the column order.
    std::vector<QVector3D> newP(nu * nv);
    std::vector<double> newW(nu * nv);
    for (int j = 0; j < nv; ++j)
    {
        int jFlip = nv - 1 - j;
        for (int i = 0; i < nu; ++i)
        {
            newP[j * nu + i] = s.P[jFlip * nu + i];
            newW[j * nu + i] = s.W[jFlip * nu + i];
        }
    }
    s.P = newP;
    s.W = newW;
}

// Helper: right-pad or truncate a string to exactly 'width' chars
inline std::string padRight(const std::string& s, int width)
{
    std::string r = s;
    while ((int)r.size() < width) r += ' ';
    return r.substr(0, width);
}

// Helper: format an integer right-justified in 'width' chars
inline std::string intField(int value, int width)
{
    std::string s = std::to_string(value);
    while ((int)s.size() < width) s = ' ' + s;
    return s;
}

// Helper: IGES Hollerith string — "5Hhello" means a 5-char string "hello"
inline std::string hollerith(const std::string& s)
{
    if (s.empty()) return "";
    return std::to_string(s.size()) + "H" + s;
}

// Write a complete IGES file from a vector of NurbsSurfaces.
// This rewrites the entire file from scratch (S/G/D/P/T sections).
inline bool writeIgesFile(const std::string& path, const std::vector<NurbsSurface>& surfaces)
{
    // Build parameter data string for each surface
    std::vector<std::string> paramStrings;
    paramStrings.reserve(surfaces.size());

    for (const auto& s : surfaces)
    {
        std::ostringstream oss;
        oss << std::setprecision(15);

        const int nu = s.K1 + 1;
        const int nv = s.K2 + 1;

        oss << "128," << s.K1 << "," << s.K2 << "," << s.M1 << "," << s.M2 << ",";
        oss << "0,0,0,0,0,";

        for (int i = 0; i < (int)s.U.size(); ++i)
            oss << s.U[i] << ",";
        for (int i = 0; i < (int)s.V.size(); ++i)
            oss << s.V[i] << ",";
        for (int i = 0; i < nu * nv; ++i)
            oss << s.W[i] << ",";
        for (int i = 0; i < nu * nv; ++i)
            oss << (double)s.P[i].x() << "," << (double)s.P[i].y() << "," << (double)s.P[i].z() << ",";

        const double uMin = s.U[s.M1];
        const double uMax = s.U[(int)s.U.size() - 1 - s.M1];
        const double vMin = s.V[s.M2];
        const double vMax = s.V[(int)s.V.size() - 1 - s.M2];
        oss << uMin << "," << uMax << "," << vMin << "," << vMax << ";";

        paramStrings.push_back(oss.str());
    }

    // Chop each parameter string into 64-char P-section lines
    struct PLineInfo {
        std::string data64;
        int dirSeq;
    };

    std::vector<PLineInfo> pLines;
    std::vector<int> pLineCounts;

    for (int eIdx = 0; eIdx < (int)paramStrings.size(); ++eIdx)
    {
        const std::string& raw = paramStrings[eIdx];
        int dirSeq = eIdx * 2 + 1;

        int lineCount = 0;
        int pos = 0;
        while (pos < (int)raw.size())
        {
            int chunkLen = std::min(64, (int)raw.size() - pos);
            std::string chunk = raw.substr(pos, chunkLen);

            PLineInfo info;
            info.data64 = chunk;
            info.dirSeq = dirSeq;
            pLines.push_back(info);
            lineCount++;
            pos += 64;
        }
        pLineCounts.push_back(lineCount);
    }

    // Assemble the full IGES file
    std::ofstream out(path);
    if (!out.is_open()) return false;

    // S section (1 line)
    {
        std::string content = "IGES file generated by NURBS Viewer";
        std::string line = padRight(content, 72) + "S" + intField(1, 7);
        out << line << "\n";
    }
    const int sCount = 1;

    // G section
    std::string gRaw;
    {
        // Extract just the filename from path
        std::string fileName = path;
        auto lastSlash = fileName.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            fileName = fileName.substr(lastSlash + 1);

        std::ostringstream g;
        g << "1H,,";
        g << "1H;,";
        g << hollerith(fileName) << ",";
        g << hollerith(fileName) << ",";
        g << hollerith("NURBS Viewer") << ",";
        g << hollerith("NURBS Viewer") << ",";
        g << "32,";
        g << "38,";
        g << "6,";
        g << "308,";
        g << "15,";
        g << hollerith(fileName) << ",";
        g << "1.0,";
        g << "2,";
        g << hollerith("MM") << ",";
        g << "1,";
        g << "0.0,";
        g << hollerith("20260311.120000") << ",";
        g << "0.001,";
        g << "10000.0,";
        g << hollerith("Author") << ",";
        g << hollerith("Organization") << ",";
        g << "11,";
        g << "0,";
        g << hollerith("20260311.120000") << ";";

        gRaw = g.str();
    }

    int gCount = 0;
    {
        int pos = 0;
        while (pos < (int)gRaw.size())
        {
            int chunkLen = std::min(72, (int)gRaw.size() - pos);
            std::string chunk = gRaw.substr(pos, chunkLen);
            gCount++;

            std::string line = padRight(chunk, 72) + "G" + intField(gCount, 7);
            out << line << "\n";
            pos += 72;
        }
    }

    // D section (2 lines per entity)
    const int dCount = (int)surfaces.size() * 2;
    int pSeqStart = 1;

    for (int eIdx = 0; eIdx < (int)surfaces.size(); ++eIdx)
    {
        int dSeq1 = eIdx * 2 + 1;
        int dSeq2 = eIdx * 2 + 2;

        // Line 1
        std::string d1;
        d1 += intField(128, 8);
        d1 += intField(pSeqStart, 8);
        d1 += intField(0, 8);
        d1 += intField(0, 8);
        d1 += intField(0, 8);
        d1 += intField(0, 8);
        d1 += intField(0, 8);
        d1 += intField(0, 8);
        d1 += padRight("00000001", 8);
        d1 += "D" + intField(dSeq1, 7);
        out << d1 << "\n";

        // Line 2
        std::string d2;
        d2 += intField(128, 8);
        d2 += intField(0, 8);
        d2 += intField(0, 8);
        d2 += intField(pLineCounts[eIdx], 8);
        d2 += intField(0, 8);
        d2 += padRight("", 8);
        d2 += padRight("", 8);
        d2 += padRight("NURBS", 8);
        d2 += intField(0, 8);
        d2 += "D" + intField(dSeq2, 7);
        out << d2 << "\n";

        pSeqStart += pLineCounts[eIdx];
    }

    // P section
    const int pCount = (int)pLines.size();
    for (int i = 0; i < pCount; ++i)
    {
        const PLineInfo& info = pLines[i];
        std::string line = padRight(info.data64, 64) + " " + intField(info.dirSeq, 7)
                           + "P" + intField(i + 1, 7);
        out << line << "\n";
    }

    // T section (1 line)
    {
        std::ostringstream t;
        t << "S" << intField(sCount, 7)
          << "G" << intField(gCount, 7)
          << "D" << intField(dCount, 7)
          << "P" << intField(pCount, 7);
        std::string line = padRight(t.str(), 72) + "T" + intField(1, 7);
        out << line << "\n";
    }

    return true;
}
