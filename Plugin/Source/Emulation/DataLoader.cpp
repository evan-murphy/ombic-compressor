#include "DataLoader.h"
#include <cmath>

namespace emulation {

static float parseFloat(const juce::String& s)
{
    return s.trim().getFloatValue();
}

static std::optional<float> parseOptionalFloat(const juce::String& s)
{
    auto t = s.trim();
    if (t.isEmpty()) return {};
    float v = t.getFloatValue();
    if (std::isnan(v) && t != "nan") return {};
    return v;
}

static std::vector<juce::String> tokenizeCsvLine(const juce::String& line)
{
    std::vector<juce::String> out;
    juce::String current;
    bool inQuotes = false;
    for (int i = 0; i < line.length(); ++i)
    {
        juce::juce_wchar c = line[i];
        if (c == '"') inQuotes = !inQuotes;
        else if ((c == ',' && !inQuotes) || c == '\r')
        {
            out.push_back(current.trim());
            current = juce::String();
        }
        else if (c != '\n')
            current += c;
    }
    if (current.isNotEmpty())
        out.push_back(current.trim());
    return out;
}

static std::vector<CompressionRow> loadCompressionCurve(const juce::File& path)
{
    std::vector<CompressionRow> rows;
    if (!path.existsAsFile()) return rows;
    juce::String content = path.loadFileAsString();
    juce::StringArray lines;
    lines.addLines(content);
    if (lines.size() < 2) return rows;
    // Header: input_db,output_db,gain_reduction_db,threshold,ratio,knee,attack_ms,release_ms
    int idxInput = -1, idxOutput = -1, idxGR = -1, idxThresh = -1, idxRatio = -1, idxAttack = -1, idxRelease = -1;
    juce::StringArray headers;
    headers.addTokens(lines[0], ",", "");
    for (int i = 0; i < headers.size(); ++i)
    {
        auto h = headers[i].toLowerCase().trim();
        if (h == "input_db") idxInput = i;
        else if (h == "output_db") idxOutput = i;
        else if (h == "gain_reduction_db") idxGR = i;
        else if (h == "threshold") idxThresh = i;
        else if (h == "ratio") idxRatio = i;
        else if (h == "attack_ms") idxAttack = i;
        else if (h == "release_ms") idxRelease = i;
    }
    for (int L = 1; L < lines.size(); ++L)
    {
        auto tokens = tokenizeCsvLine(lines[L]);
        if (tokens.empty()) continue;
        CompressionRow r;
        if (idxInput >= 0 && idxInput < (int)tokens.size()) r.inputDb = parseFloat(tokens[(size_t)idxInput]);
        if (idxOutput >= 0 && idxOutput < (int)tokens.size()) r.outputDb = parseFloat(tokens[(size_t)idxOutput]);
        if (idxGR >= 0 && idxGR < (int)tokens.size()) r.gainReductionDb = parseFloat(tokens[(size_t)idxGR]);
        if (idxThresh >= 0 && idxThresh < (int)tokens.size()) r.threshold = parseOptionalFloat(tokens[(size_t)idxThresh]);
        if (idxRatio >= 0 && idxRatio < (int)tokens.size()) r.ratio = parseOptionalFloat(tokens[(size_t)idxRatio]);
        if (idxAttack >= 0 && idxAttack < (int)tokens.size()) r.attackMs = parseOptionalFloat(tokens[(size_t)idxAttack]);
        if (idxRelease >= 0 && idxRelease < (int)tokens.size()) r.releaseMs = parseOptionalFloat(tokens[(size_t)idxRelease]);
        rows.push_back(r);
    }
    return rows;
}

static std::vector<TimingRow> loadTimingCsv(const juce::File& path)
{
    std::vector<TimingRow> rows;
    if (!path.existsAsFile()) return rows;
    juce::String content = path.loadFileAsString();
    juce::StringArray lines;
    lines.addLines(content);
    if (lines.size() < 2) return rows;
    juce::StringArray headers;
    headers.addTokens(lines[0], ",", "");
    int idxAP = -1, idxRP = -1, idxThresh = -1, idxRatio = -1, idxAtMs = -1, idxReMs = -1;
    for (int i = 0; i < headers.size(); ++i)
    {
        auto h = headers[i].toLowerCase().trim();
        if (h == "attack_param") idxAP = i;
        else if (h == "release_param") idxRP = i;
        else if (h == "threshold") idxThresh = i;
        else if (h == "ratio") idxRatio = i;
        else if (h == "attack_time_ms") idxAtMs = i;
        else if (h == "release_time_ms") idxReMs = i;
    }
    for (int L = 1; L < lines.size(); ++L)
    {
        auto tokens = tokenizeCsvLine(lines[L]);
        if (tokens.empty()) continue;
        TimingRow r;
        if (idxAP >= 0 && idxAP < (int)tokens.size()) r.attackParam = parseOptionalFloat(tokens[(size_t)idxAP]);
        if (idxRP >= 0 && idxRP < (int)tokens.size()) r.releaseParam = parseOptionalFloat(tokens[(size_t)idxRP]);
        if (idxThresh >= 0 && idxThresh < (int)tokens.size()) r.threshold = parseOptionalFloat(tokens[(size_t)idxThresh]);
        if (idxRatio >= 0 && idxRatio < (int)tokens.size()) r.ratio = parseOptionalFloat(tokens[(size_t)idxRatio]);
        if (idxAtMs >= 0 && idxAtMs < (int)tokens.size()) r.attackTimeMs = parseOptionalFloat(tokens[(size_t)idxAtMs]);
        if (idxReMs >= 0 && idxReMs < (int)tokens.size()) r.releaseTimeMs = parseOptionalFloat(tokens[(size_t)idxReMs]);
        rows.push_back(r);
    }
    return rows;
}

static std::vector<FRRow> loadFrequencyResponse(const juce::File& path)
{
    std::vector<FRRow> rows;
    if (!path.existsAsFile()) return rows;
    juce::String content = path.loadFileAsString();
    juce::StringArray lines;
    lines.addLines(content);
    if (lines.size() < 2) return rows;
    juce::StringArray headers;
    headers.addTokens(lines[0], ",", "");
    int idxFreq = -1, idxMag = -1, idxDrive = -1;
    for (int i = 0; i < headers.size(); ++i)
    {
        auto h = headers[i].toLowerCase().trim();
        if (h == "frequency_hz") idxFreq = i;
        else if (h == "magnitude_db") idxMag = i;
        else if (h == "drive_level_db") idxDrive = i;
    }
    for (int L = 1; L < lines.size(); ++L)
    {
        auto tokens = tokenizeCsvLine(lines[L]);
        if (tokens.empty()) continue;
        FRRow r;
        if (idxFreq >= 0 && idxFreq < (int)tokens.size()) r.frequencyHz = parseOptionalFloat(tokens[(size_t)idxFreq]);
        if (idxMag >= 0 && idxMag < (int)tokens.size()) r.magnitudeDb = parseOptionalFloat(tokens[(size_t)idxMag]);
        if (idxDrive >= 0 && idxDrive < (int)tokens.size()) r.driveLevelDb = parseOptionalFloat(tokens[(size_t)idxDrive]);
        rows.push_back(r);
    }
    return rows;
}

static std::vector<THDRow> loadThdJson(const juce::File& path)
{
    std::vector<THDRow> rows;
    if (!path.existsAsFile()) return rows;
    juce::var json;
    if (!juce::JSON::parse(path.loadFileAsString(), json).wasOk() || !json.isArray()) return rows;
    const auto* arr = json.getArray();
    if (!arr) return rows;
    for (const auto& item : *arr)
    {
        if (!item.isObject()) continue;
        auto* obj = item.getDynamicObject();
        if (!obj) continue;
        THDRow r;
        if (obj->hasProperty("level_db")) r.levelDb = (float)obj->getProperty("level_db");
        if (obj->hasProperty("thd_percent")) r.thdPercent = (float)obj->getProperty("thd_percent");
        rows.push_back(r);
    }
    return rows;
}

AnalyzerOutput loadAnalyzerOutput(const juce::File& outputDir)
{
    AnalyzerOutput out;
    out.compressionRows = loadCompressionCurve(outputDir.getChildFile("compression_curve.csv"));
    out.timingRows = loadTimingCsv(outputDir.getChildFile("timing.csv"));
    out.frRows = loadFrequencyResponse(outputDir.getChildFile("frequency_response.csv"));
    out.thdRows = loadThdJson(outputDir.getChildFile("thd_vs_level.json"));
    return out;
}

} // namespace emulation
