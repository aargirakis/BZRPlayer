/**
 * 
 * @file
 *
 * @brief PagerAdapter implementation over layout-defined ViewPager content
 *
 * @author vitamin.caig@gmail.com
 * 
 */

package app.zxtune.ui;

import android.content.Context;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.PagerTabStrip;
import android.support.v4.view.ViewPager;
import android.view.View;
import android.view.ViewGroup;

public class ViewPagerAdapter extends PagerAdapter {

  private final int delta;
  private final String[] titles;

  public ViewPagerAdapter(ViewPager pager) {
    this.delta = pager.getChildAt(0) instanceof PagerTabStrip ? 1 : 0;
    this.titles = new String[pager.getChildCount() - delta];
    for (int i = 0; i != titles.length; ++i) {
      titles[i] = getTitle(pager.getContext(), pager.getChildAt(delta + i)); 
    }
    pager.setOffscreenPageLimit(titles.length);
  }

  private String getTitle(Context ctx, View pane) {
    //only String can be stored in View's tag, so try to decode manually
    final String ID_PREFIX = "@";
    final String tag = (String) pane.getTag();
    return tag.startsWith(ID_PREFIX)
      ? getStringByName(ctx, tag.substring(ID_PREFIX.length()))
      : tag;
  }
  
  private String getStringByName(Context ctx, String name) {
    final int id = ctx.getResources().getIdentifier(name, null, ctx.getPackageName());
    return ctx.getString(id);
  }
  
  @Override
  public int getCount() {
    return titles.length;
  }

  @Override
  public boolean isViewFromObject(View view, Object object) {
    return view == object;
  }

  @Override
  public Object instantiateItem(ViewGroup container, int position) {
    return container.getChildAt(delta + position);
  }

  @Override
  public void destroyItem(ViewGroup container, int position, Object object) {
  }
  
  @Override
  public CharSequence getPageTitle(int position) {
    return titles[position]; 
  }
}
