//cc by-sa 4.0 license
//bitluni
#pragma once

class Ray
{
  public:
  Ray(Vector pos, Vector dir)
  :p(pos), d(dir)
  {
  }
  Vector p;
  Vector d;
};

class Raytracable
{
  public:
  float reflection;
  Vector c;
  Raytracable()
  {
    reflection = 0;
  }
  virtual bool intersection(Ray &ray, Vector &i, float &t) const = 0;
  virtual Vector normal(Vector &i) const = 0;
  virtual Vector color(Vector &p) const = 0;
};

class Sphere : public Raytracable
{
  public:
  float r;
  float r2;
  Vector p;
  Sphere(Vector pos, float radius)
    :p(pos),
     r(radius),
     r2(radius * radius)
  { 
  }

  virtual bool intersection(Ray &ray, Vector &i, float &t) const
  {
    Vector L = p - ray.p;
    float tca = L.dot(ray.d);
    if(tca < 0) return false;
    float d2 = L.dot(L) - tca * tca;
    if (d2 >= r2) return false;
    float thc = Vector::sqrt(r2 - d2);
    float ct = tca - thc;
    if(t <= ct) return false;
    t = ct;
    i = ray.p + ray.d * ct;
    return true;
  }
  
  virtual Vector normal(Vector &i) const
  {
    return (i - p) * (1.f / r);
  }
  
  virtual Vector color(Vector &p) const
  {
    return c;
  }
};

class Checker : public Raytracable
{
  public:
  Checker()
  {
  }
  
  virtual bool intersection(Ray &ray, Vector &i, float &t) const
  {
    if(ray.d[1] >= 0 || ray.p[1] <= 0) return false;
    float ct = ray.p[1] / -ray.d[1];
    if(ct >= t) return false;
    i = Vector(ray.p[0] + ray.d[0] * ct, 0, ray.p[2] + ray.d[2] * ct);
    t = ct;
    return true;
  }
  
  virtual Vector normal(Vector &i) const
  {
    return Vector(0, 1, 0);
  }
  
  virtual Vector color(Vector &p) const
  {
    float c = ((int)p[0] + (int)p[2] + (p[0] >= 0 ? 1 : 0)) & 1;
    return Vector(0.8 + 0.2 * c, c, c);
  }
};

const float FAR = 10000;
Vector raytrace(Raytracable **objects, int count, Ray &r, Vector &light, int depth, Raytracable *self = 0)
{
  if(depth == 0)
    return Vector(0, 0, 0);
  Vector i;
  float t = FAR;
  Raytracable *best = 0;
  for(int n = 0; n < count; n++)
  {
    Raytracable *o = objects[n];
    if(o != self && o->intersection(r, i, t))
      best = o;
  }
  float fog = t * 0.02f;
  float fc = 0.5f - (r.d[1] < 0 ? 0 : r.d[1]) * 0.5;
  Vector fogc = Vector(fc, fc, 1.0f);
  if(fog >= 1) return fogc;
  if(!best)
  {
    return fogc;    
  }
  Vector n = best->normal(i);
  float l = light.dot(n) * 0.9;
  if(l < 0) 
    l = 0;
  else
  {
    Ray r2(i, light);
    Vector i2;
    float t2 = FAR;
    for(int n = 0; n < count; n++)
    {
      Raytracable *o = objects[n];
      if(o == best) continue;
      if(o->intersection(r2, i2, t2))
      {
        l = 0;
        break;
      }
    }
  }
  Vector c = (best->color(i) * (0.1f + l)) * (1 - fog) + fogc * fog;
  if(best->reflection == 0)
    return c;
  float dn = r.d.dot(n);
  float fr = (0.2f + (1+dn) * 0.8f) * best->reflection;
  if(fr < 0) fr = 0;
  Vector refl = r.d - n * (dn * 2);
  Ray nr = Ray(i, refl);
  //return Vector(fr, fr, fr);
  c = raytrace(objects, count, nr, light, depth - 1, best) * fr + c * (1 - fr);
  return c;
}
