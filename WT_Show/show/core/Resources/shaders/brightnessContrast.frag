#version 330 core
// this flag indicated the object segmentation (background removal) is enabled and that we should
// be updating the alpha to match the objSegMaskTexture alpha mask.
uniform bool objSegEnabled;

// this is the texture containing the alpha mask given by the object segmentation (background removal)
// functionality.
uniform sampler2D objSegMaskTexture;

// this flag indicated we want to change the color of pixels indicated in the objSegMaskTexture as
// background so we have a preview of what would be made transparent when objSegEnabled is enabled.
// Notice that enabling objSegEnabled ignored showObjSegPreview;
uniform bool showObjSegPreview;
uniform sampler2D objSegPreviewTexture;

uniform sampler2D _MainTexture;
uniform sampler2D imgHistTexture;
uniform float transparency;
uniform float contrast;
uniform vec3 hsl;
uniform bool colorizeEnabled;
uniform vec3 colorizeHsl;
uniform bool magicFixEnabled;
uniform bool sepiaEnabled;
uniform float imgWidth;
uniform float imgHeight;
uniform float sharpen;
uniform float histParam;

out vec4 fragColor;
varying highp vec4 color;
varying highp vec2 textureCoord;
varying highp vec3 normal;

float normpdf(in float x, in float sigma);
float normpdf3(in vec3 v, in float sigma);
vec4 bilateralFilter(vec4 pixelColor);
vec4 brigtnessContrastFilter(vec4 pixelColor);
vec3 RGBtoHSL(vec3 color);
float hueToIntensity(float v1, float v2, float h);
vec3 HSLtoRGB(vec3 color);
vec3 RGBtoHSV(vec3 color);
vec3 HSVtoRGB(vec3 color);
vec4 matchHistFilter(vec4 pixelColor);
vec4 EqualHist(vec4 pixelColor);
vec4 hueSaturationFilter(vec4 pixelColor);
float RGBtoL(vec3 color);
vec4 sharpenFilter(vec4 pixelColor);

#define MAX_SHARPEN_VALUE   1.0f

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 5

float normpdf(in float x, in float sigma)
{
    return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in vec3 v, in float sigma)
{
    return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

vec3 matchHistFilter(vec3 pixelColor)
{
    if(magicFixEnabled==false)
        return pixelColor;
    vec3 hsv=RGBtoHSV(pixelColor);
    float gray=hsv.z;
    float Temp=0.0;
    float target;
    float j=1.0/2.0;
    Temp=texture2D(imgHistTexture, vec2((gray*255.0*3.0+1.0)/(256.0*3.0-1.0),j)).r;
    target=gray+histParam*(Temp-gray);
    if(target>1.0)
        target=1.0;
    if(target<0.0)
        target=0.0;
    pixelColor.rgb=HSVtoRGB(vec3(hsv.x,hsv.y,target));
    return vec3(pixelColor.rgb);
}

vec4 bilateralFilter(vec4 pixelColor, vec2 pos)
{
    /* declare stuff */
    const int kSize = (MSIZE-1)/2;
    float kernel[MSIZE];
    vec3 final_colour = vec3(0.0);
    vec2 sketchSize = vec2(imgWidth, imgHeight);
    vec3 c = matchHistFilter(pixelColor.rgb);
    float step_w = 1.0/imgWidth;
    float step_h = 1.0/imgHeight;

    /* create the 1-D kernel */
    float Z = 0.0;
    kernel[0] = kernel[4] = 0.039104;
    kernel[1] = kernel[3] = 0.039695;
    kernel[2] = 0.039894;

    /*
    for (int j = 0; j <= kSize; ++j)
    {
        kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), SIGMA);
    }
    */

    vec3 cc;
    float factor;
    float bZ = 1.0/normpdf(0.0, BSIGMA);

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(-step_w, -step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize - 1]*kernel[kSize - 1];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(0, -1 * step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize - 1]*kernel[kSize];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(step_w, -step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize - 1]*kernel[kSize + 1];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(-step_w, 0)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize]*kernel[kSize - 1];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(0, 0)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize]*kernel[kSize];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(step_w, 0)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize]*kernel[kSize + 1];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(-step_w, step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize + 1]*kernel[kSize - 1];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(0, step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize + 1]*kernel[kSize];
    Z += factor;
    final_colour += factor*cc;

    cc = matchHistFilter(texture2D(_MainTexture, pos + vec2(step_w, step_h)).rgb);
    factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize + 1]*kernel[kSize + 1];
    Z += factor;
    final_colour += factor*cc;

    return vec4(final_colour/Z, pixelColor.a);
}

vec3 RGBtoHSV(vec3 color)
{
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float h = 0.0;
    float s = 0.0;
    float v=cmax;
    float diff=cmax-cmin;
    if(cmax!=0.0)
        s=diff/cmax;
    else
        return vec3(0.0);
    if(color.r==cmax)
        h=(color.g-color.b)/diff;
    else if(color.g==cmax)
        h=2.0+(color.b-color.r)/diff;
    else
        h=4.0+(color.r-color.g)/diff;
    h=h*60.0;
    if(h<0.0)
        h=h+360.0;
    h=h/360.0;
    return vec3(h, s, v);

}

vec3 HSVtoRGB(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


vec4 brigtnessContrastFilter(vec4 pixelColor)
{
    if(contrast == 0.0)
        return pixelColor;

    float brightness = 0.1;
    pixelColor.rgb /= max(1.0/256.0, pixelColor.a);
    float c = 1.0 + contrast;
    float contrastGainFactor = 1.0 + c * c * c * c * step(0.0, contrast);
    pixelColor.rgb = ((pixelColor.rgb - 0.5) * (contrastGainFactor * contrast + 1.0)) + 0.5;
    pixelColor.rgb = mix(pixelColor.rgb, vec3(step(0.0, brightness)), abs(brightness));

    return pixelColor;
}

vec3 RGBtoHSL(vec3 color) {
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float h = 0.0;
    float s = 0.0;
    float l = (cmin + cmax) / 2.0;
    float diff = cmax - cmin;
    if (diff > 1.0 / 256.0) {
        if (l < 0.5)
            s = diff / (cmin + cmax);
        else
            s = diff / (2.0 - (cmin + cmax));
        if (color.r == cmax)
            h = (color.g - color.b) / diff;
        else if (color.g == cmax)
            h = 2.0 + (color.b - color.r) / diff;
        else
            h = 4.0 + (color.r - color.g) / diff;
        h /= 6.0;
    }
    return vec3(h, s, l);
}

float hueToIntensity(float v1, float v2, float h) {
    h = fract(h);
    if (h < 1.0 / 6.0)
        return v1 + (v2 - v1) * 6.0 * h;
    else if (h < 1.0 / 2.0)
        return v2;
    else if (h < 2.0 / 3.0)
        return v1 + (v2 - v1) * 6.0 * (2.0 / 3.0 - h);
    return v1;
}

vec3 HSLtoRGB(vec3 color) {
    float h = color.x;
    float l = color.z;
    float s = color.y;

    if (s < 1.0 / 256.0) return vec3(l);

    float v1;
    float v2;

    if (l < 0.5) v2 = l * (1.0 + s);
    else v2 = (l + s) - (s * l);

    v1 = 2.0 * l - v2;
    float d = 1.0 / 3.0;
    float r = hueToIntensity(v1, v2, h + d);
    float g = hueToIntensity(v1, v2, h);
    float b = hueToIntensity(v1, v2, h - d);
    return vec3(r, g, b);
}


vec4 hueSaturationFilter(vec4 pixelColor)
{
    pixelColor = vec4(pixelColor.rgb / max(1.0/256.0, pixelColor.a), pixelColor.a);
    pixelColor.rgb = mix(vec3(dot(pixelColor.rgb, vec3(0.2125, 0.7154, 0.0721))), pixelColor.rgb, 1.0 + hsl.y);
    pixelColor.xyz = RGBtoHSL(pixelColor.rgb);
    if(sepiaEnabled)
    {
        pixelColor.x = hsl.x;
    }
    else
    {
        pixelColor.x = (pixelColor.x + hsl.x);
    }
    pixelColor.rgb = HSLtoRGB(vec3(pixelColor.x, pixelColor.y, pixelColor.z));
    pixelColor.rgb = pixelColor.rgb * (1.0 + hsl.z);
    return vec4(pixelColor.rgb * pixelColor.a, pixelColor.a);
}

float RGBtoL(vec3 color) {
    float cmin = min(color.r, min(color.g, color.b));
    float cmax = max(color.r, max(color.g, color.b));
    float l = (cmin + cmax) / 2.0;
    return l;
}

vec4 sharpenFilter(vec4 pixelColor)
{
    vec4 sum = vec4(0.0);

    float step_w = 1.0/imgWidth;
    float step_h = 1.0/imgHeight;
    float kernel[9];
    vec2 offset[9];
    int i;
    int j;

    if(sharpen == 0)
    {
        pixelColor.rgb=matchHistFilter(pixelColor.rgb);
        return pixelColor;
    }

    offset[0] = vec2(-step_w, -step_h);
    offset[1] = vec2(0.0, -step_h);
    offset[2] = vec2(step_w, -step_h);
    offset[3] = vec2(-step_w, 0.0);
    offset[4] = vec2(0.0, 0.0);
    offset[5] = vec2(step_w, 0.0);
    offset[6] = vec2(-step_w, step_h);
    offset[7] = vec2(0.0, step_h);
    offset[8] = vec2(step_w, step_h);

    /* SHARPEN KERNEL
     *            0              -1*sharpen      0
     *           -1*sharpen      4*sharpen+1     -1*sharpen
     *            0              -1*sharpen      0
     */
    kernel[0] = 0.;
    kernel[1] = -1. * sharpen * MAX_SHARPEN_VALUE / 100.0f;
    kernel[2] = 0.;
    kernel[3] = -1. * sharpen * MAX_SHARPEN_VALUE / 100.0f;
    kernel[4] = 4. * sharpen * MAX_SHARPEN_VALUE / 100.0f + 1.;
    kernel[5] = -1. * sharpen * MAX_SHARPEN_VALUE / 100.0f;
    kernel[6] = 0.;
    kernel[7] = -1. * sharpen * MAX_SHARPEN_VALUE / 100.0f;
    kernel[8] = 0.;

    vec2 pos;
    for (i = 0; i < 9; i++)
    {
        pos = textureCoord.st + offset[i];
        vec4 color = texture2D(_MainTexture, pos);

        /*
         * Before applying the sharpen algorithem, we must proces the pixel color firstly.
         */
        color = bilateralFilter(color, pos);

        sum += color * kernel[i];
    }

    return sum;
}

void main(void)
{
    vec4 pixelColor = texture(_MainTexture, textureCoord);
    pixelColor = sharpenFilter(pixelColor);
    pixelColor = brigtnessContrastFilter(pixelColor);
    pixelColor = hueSaturationFilter(pixelColor);
    pixelColor = vec4(pixelColor.rgb, pixelColor.a * min(transparency, 1.0));
    pixelColor = clamp(pixelColor, 0.0, 1.0);

    if(objSegEnabled) {
        pixelColor.a = pixelColor.a * texture(objSegMaskTexture, textureCoord).r;
    }
    else if(showObjSegPreview) {
        float factor = texture(objSegPreviewTexture, textureCoord).r;
        float mixFactor = 1.0 - factor;
        pixelColor.rgb = (pixelColor.rgb + vec3(1 * mixFactor, 0, 0)) / (2.0 - factor);
    }

    gl_FragColor = vec4(pixelColor.rgb, pixelColor.a);
}
